/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */


#include "NativeFeatureIncludes.h"
#if _RAKNET_SUPPORT_DirectoryDeltaTransfer == 1 && _RAKNET_SUPPORT_FileOperations == 1

#include "BitStream.h"
#include "DirectoryDeltaTransfer.h"
#include "FileList.h"
#include "FileListTransfer.h"
#include "FileListTransferCBInterface.h"
#include "FileOperations.h"
#include "IncrementalReadInterface.h"
#include "MessageIdentifiers.h"
#include "RakPeerInterface.h"
#include "StringCompressor.h"

using namespace RakNet;

#ifdef _MSC_VER
#pragma warning(push)
#endif

class DDTCallback : public FileListTransferCBInterface {
public:
    unsigned                     subdirLen;
    char                         outputSubdir[512];
    FileListTransferCBInterface* onFileCallback;

    DDTCallback() {}
    virtual ~DDTCallback() {}

    virtual bool OnFile(OnFileStruct* onFileStruct) {
        char fullPathToDir[1024];

        if (onFileStruct->fileName && onFileStruct->fileData && subdirLen < strlen(onFileStruct->fileName)) {
            strcpy(fullPathToDir, outputSubdir);
            strcat(fullPathToDir, onFileStruct->fileName + subdirLen);
            WriteFileWithDirectories(
                fullPathToDir,
                (char*)onFileStruct->fileData,
                (unsigned int)onFileStruct->byteLengthOfThisFile
            );
        } else fullPathToDir[0] = 0;

        return onFileCallback->OnFile(onFileStruct);
    }

    virtual void OnFileProgress(FileProgressStruct* fps) {
        char fullPathToDir[1024];

        if (fps->onFileStruct->fileName && subdirLen < strlen(fps->onFileStruct->fileName)) {
            strcpy(fullPathToDir, outputSubdir);
            strcat(fullPathToDir, fps->onFileStruct->fileName + subdirLen);
        } else fullPathToDir[0] = 0;

        onFileCallback->OnFileProgress(fps);
    }
    virtual bool OnDownloadComplete(DownloadCompleteStruct* dcs) { return onFileCallback->OnDownloadComplete(dcs); }
};

STATIC_FACTORY_DEFINITIONS(DirectoryDeltaTransfer, DirectoryDeltaTransfer);

DirectoryDeltaTransfer::DirectoryDeltaTransfer() {
    applicationDirectory[0]  = 0;
    fileListTransfer         = 0;
    availableUploads         = RakNet::OP_NEW<FileList>(_FILE_AND_LINE_);
    priority                 = HIGH_PRIORITY;
    orderingChannel          = 0;
    incrementalReadInterface = 0;
}
DirectoryDeltaTransfer::~DirectoryDeltaTransfer() { RakNet::OP_DELETE(availableUploads, _FILE_AND_LINE_); }
void DirectoryDeltaTransfer::SetFileListTransferPlugin(FileListTransfer* flt) {
    if (fileListTransfer) {
        DataStructures::List<FileListProgress*> fileListProgressList;
        fileListTransfer->GetCallbacks(fileListProgressList);
        unsigned int i;
        for (i = 0; i < fileListProgressList.Size(); i++) availableUploads->RemoveCallback(fileListProgressList[i]);
    }

    fileListTransfer = flt;

    if (flt) {
        DataStructures::List<FileListProgress*> fileListProgressList;
        flt->GetCallbacks(fileListProgressList);
        unsigned int i;
        for (i = 0; i < fileListProgressList.Size(); i++) availableUploads->AddCallback(fileListProgressList[i]);
    } else {
        availableUploads->ClearCallbacks();
    }
}
void DirectoryDeltaTransfer::SetApplicationDirectory(const char* pathToApplication) {
    if (pathToApplication == 0 || pathToApplication[0] == 0) applicationDirectory[0] = 0;
    else {
        strncpy(applicationDirectory, pathToApplication, 510);
        if (applicationDirectory[strlen(applicationDirectory) - 1] != '/'
            && applicationDirectory[strlen(applicationDirectory) - 1] != '\\')
            strcat(applicationDirectory, "/");
        applicationDirectory[511] = 0;
    }
}
void DirectoryDeltaTransfer::SetUploadSendParameters(PacketPriority _priority, char _orderingChannel) {
    priority        = _priority;
    orderingChannel = _orderingChannel;
}
void DirectoryDeltaTransfer::AddUploadsFromSubdirectory(const char* subdir) {
    availableUploads
        ->AddFilesFromDirectory(applicationDirectory, subdir, true, false, true, FileListNodeContext(0, 0, 0, 0));
}
unsigned short DirectoryDeltaTransfer::DownloadFromSubdirectory(
    FileList&                    localFiles,
    const char*                  subdir,
    const char*                  outputSubdir,
    bool                         prependAppDirToOutputSubdir,
    SystemAddress                host,
    FileListTransferCBInterface* onFileCallback,
    PacketPriority               _priority,
    char                         _orderingChannel,
    FileListProgress*            cb
) {
    RakAssert(host != UNASSIGNED_SYSTEM_ADDRESS);

    DDTCallback* transferCallback;

    localFiles.AddCallback(cb);

    // Prepare the callback data
    transferCallback = RakNet::OP_NEW<DDTCallback>(_FILE_AND_LINE_);
    if (subdir && subdir[0]) {
        transferCallback->subdirLen = (unsigned int)strlen(subdir);
        if (subdir[transferCallback->subdirLen - 1] != '/' && subdir[transferCallback->subdirLen - 1] != '\\')
            transferCallback->subdirLen++;
    } else transferCallback->subdirLen = 0;
    if (prependAppDirToOutputSubdir) strcpy(transferCallback->outputSubdir, applicationDirectory);
    else transferCallback->outputSubdir[0] = 0;
    if (outputSubdir) strcat(transferCallback->outputSubdir, outputSubdir);
    if (transferCallback->outputSubdir[strlen(transferCallback->outputSubdir) - 1] != '/'
        && transferCallback->outputSubdir[strlen(transferCallback->outputSubdir) - 1] != '\\')
        strcat(transferCallback->outputSubdir, "/");
    transferCallback->onFileCallback = onFileCallback;

    // Setup the transfer plugin to get the response to this download request
    unsigned short setId = fileListTransfer->SetupReceive(transferCallback, true, host);

    // Send to the host, telling it to process this request
    RakNet::BitStream outBitstream;
    outBitstream.Write((MessageID)ID_DDT_DOWNLOAD_REQUEST);
    outBitstream.Write(setId);
    StringCompressor::Instance()->EncodeString(subdir, 256, &outBitstream);
    StringCompressor::Instance()->EncodeString(outputSubdir, 256, &outBitstream);
    localFiles.Serialize(&outBitstream);
    SendUnified(&outBitstream, _priority, RELIABLE_ORDERED, _orderingChannel, host, false);

    return setId;
}
unsigned short DirectoryDeltaTransfer::DownloadFromSubdirectory(
    const char*                  subdir,
    const char*                  outputSubdir,
    bool                         prependAppDirToOutputSubdir,
    SystemAddress                host,
    FileListTransferCBInterface* onFileCallback,
    PacketPriority               _priority,
    char                         _orderingChannel,
    FileListProgress*            cb
) {
    FileList localFiles;
    // Get a hash of all the files that we already have (if any)
    localFiles.AddFilesFromDirectory(
        prependAppDirToOutputSubdir ? applicationDirectory : 0,
        outputSubdir,
        true,
        false,
        true,
        FileListNodeContext(0, 0, 0, 0)
    );
    return DownloadFromSubdirectory(
        localFiles,
        subdir,
        outputSubdir,
        prependAppDirToOutputSubdir,
        host,
        onFileCallback,
        _priority,
        _orderingChannel,
        cb
    );
}
void DirectoryDeltaTransfer::GenerateHashes(
    FileList&   localFiles,
    const char* outputSubdir,
    bool        prependAppDirToOutputSubdir
) {
    localFiles.AddFilesFromDirectory(
        prependAppDirToOutputSubdir ? applicationDirectory : 0,
        outputSubdir,
        true,
        false,
        true,
        FileListNodeContext(0, 0, 0, 0)
    );
}
void DirectoryDeltaTransfer::ClearUploads(void) { availableUploads->Clear(); }
void DirectoryDeltaTransfer::OnDownloadRequest(Packet* packet) {
    char              subdir[256];
    char              remoteSubdir[256];
    RakNet::BitStream inBitstream(packet->data, packet->length, false);
    FileList          remoteFileHash;
    FileList          delta;
    unsigned short    setId;
    inBitstream.IgnoreBits(8);
    inBitstream.Read(setId);
    StringCompressor::Instance()->DecodeString(subdir, 256, &inBitstream);
    StringCompressor::Instance()->DecodeString(remoteSubdir, 256, &inBitstream);
    if (remoteFileHash.Deserialize(&inBitstream) == false) {
#ifdef _DEBUG
        RakAssert(0);
#endif
        return;
    }

    availableUploads->GetDeltaToCurrent(&remoteFileHash, &delta, subdir, remoteSubdir);
    if (incrementalReadInterface == 0) delta.PopulateDataFromDisk(applicationDirectory, true, false, true);
    else delta.FlagFilesAsReferences();

    // This will call the ddtCallback interface that was passed to FileListTransfer::SetupReceive on the remote system
    fileListTransfer->Send(
        &delta,
        mRakPeerInterface,
        packet->systemAddress,
        setId,
        priority,
        orderingChannel,
        incrementalReadInterface,
        chunkSize
    );
}
PluginReceiveResult DirectoryDeltaTransfer::OnReceive(Packet* packet) {
    switch (packet->data[0]) {
    case ID_DDT_DOWNLOAD_REQUEST:
        OnDownloadRequest(packet);
        return RR_STOP_PROCESSING_AND_DEALLOCATE;
    }

    return RR_CONTINUE_PROCESSING;
}

unsigned DirectoryDeltaTransfer::GetNumberOfFilesForUpload(void) const { return availableUploads->fileList.Size(); }

void DirectoryDeltaTransfer::SetDownloadRequestIncrementalReadInterface(
    IncrementalReadInterface* _incrementalReadInterface,
    unsigned int              _chunkSize
) {
    incrementalReadInterface = _incrementalReadInterface;
    chunkSize                = _chunkSize;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif // _RAKNET_SUPPORT_*
