/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

/// \file
/// \brief \b [Internal] A single node in the Huffman Encoding Tree.
///

#ifndef __HUFFMAN_ENCODING_TREE_NODE
#define __HUFFMAN_ENCODING_TREE_NODE

struct HuffmanEncodingTreeNode {
    unsigned char            value;
    unsigned                 weight;
    HuffmanEncodingTreeNode* left;
    HuffmanEncodingTreeNode* right;
    HuffmanEncodingTreeNode* parent;
};

#endif
