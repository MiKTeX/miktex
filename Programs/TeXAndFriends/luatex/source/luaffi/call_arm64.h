/*
** This file has been pre-processed with DynASM.
** http://luajit.org/dynasm.html
** DynASM version 1.4.0, DynASM arm version 1.4.0
** DO NOT EDIT! The original file is in "call_arm64.dasc".
*/

/* vim: ts=4 sw=4 sts=4 et tw=78
 * Portions copyright (c) 2015-present, Facebook, Inc. All rights reserved.
 * Portions copyright (c) 2011 James R. McKaskill.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#if DASM_VERSION != 10400
#error "Version mismatch between DynASM and included encoding engine"
#endif

static const unsigned int build_actionlist[1117] = {
0xf84003e1,
0x000f0000,
0x00000000,
0xf8400681,
0x000a812c,
0x00000000,
0x6d400fe2,
0x000a8cef,
0x00000000,
0xfc4003e2,
0x000f0000,
0x00000000,
0x6d4007e0,
0x000a8cef,
0x00000000,
0xfc4003e0,
0x000f0000,
0x00000000,
0x2cc10680,
0x2cc10e82,
0x00000000,
0x6cc10680,
0x6cc10e82,
0x00000000,
0x2cc10680,
0xbc408682,
0x00000000,
0x6cc10680,
0xfc408682,
0x00000000,
0x2cc10680,
0x00000000,
0x6cc10680,
0x00000000,
0xbc408680,
0x00000000,
0xfc408680,
0x00000000,
0xf81f0fe8,
0x00000000,
0xd10283ff,
0x00000000,
0xd10083ff,
0x00000000,
0xa9091fe6,
0x00000000,
0xa90817e4,
0x00000000,
0xa9070fe2,
0x00000000,
0xa90607e0,
0x00000000,
0x6d051fe6,
0x00000000,
0x6d0417e4,
0x00000000,
0x6d030fe2,
0x00000000,
0x6d0207e0,
0x00000000,
0xa90157f4,
0xa9007bfd,
0x00000000,
0x910003f4,
0x000c0000,
0x00000000,
0x52800015,
0x000a0205,
0xf2a00015,
0x000a0205,
0xf2c00015,
0x000a0205,
0xf2e00015,
0x000a0205,
0x52800002,
0x000a0205,
0x72a00002,
0x000a0205,
0x52800001,
0x000a0205,
0x72a00001,
0x000a0205,
0xaa1503e0,
0x94000000,
0x00030000,
0x00000000,
0x52800002,
0x000a0205,
0x92800001,
0xaa1503e0,
0x94000000,
0x00030000,
0x00000000,
0x52800002,
0x000a0205,
0x92800001,
0x000a0205,
0xaa1503e0,
0x94000000,
0x00030000,
0x52800002,
0x000a0205,
0xf2a00002,
0x000a0205,
0xf2c00002,
0x000a0205,
0xf2e00002,
0x000a0205,
0x12800001,
0xaa1503e0,
0x94000000,
0x00030001,
0x00000000,
0xf9000001,
0x12800021,
0xaa1503e0,
0x94000000,
0x00030002,
0x00000000,
0x52800002,
0x000a0205,
0x92800001,
0x000a0205,
0xaa1503e0,
0x94000000,
0x00030000,
0x52800002,
0x000a0205,
0xf2a00002,
0x000a0205,
0xf2c00002,
0x000a0205,
0xf2e00002,
0x000a0205,
0x12800001,
0xaa1503e0,
0x94000000,
0x00030001,
0x00000000,
0x2d010c02,
0x00000000,
0x6d010c02,
0x00000000,
0xbd000802,
0x00000000,
0xfd000802,
0x00000000,
0x2d000400,
0x00000000,
0x6d000400,
0x00000000,
0xbd000000,
0x00000000,
0xfd000000,
0x00000000,
0x52800002,
0x000a0205,
0xf2a00002,
0x000a0205,
0xf2c00002,
0x000a0205,
0xf2e00002,
0x000a0205,
0x52800009,
0x000a0205,
0xf2a00009,
0x000a0205,
0xf2c00009,
0x000a0205,
0xf2e00009,
0x000a0205,
0xd63f0120,
0x00000000,
0xa9400be1,
0x000a8cef,
0xa9000801,
0x00000000,
0xf84003e1,
0x000f0000,
0xf9000001,
0x00000000,
0xa8c10a81,
0xa9000801,
0x00000000,
0xf8408681,
0xf9000001,
0x00000000,
0x12800021,
0xaa1503e0,
0x94000000,
0x00030002,
0x00000000,
0x52800002,
0x000a0205,
0x92800001,
0x000a0205,
0xaa1503e0,
0x94000000,
0x00030000,
0x52800002,
0x000a0205,
0xf2a00002,
0x000a0205,
0xf2c00002,
0x000a0205,
0xf2e00002,
0x000a0205,
0x12800001,
0xaa1503e0,
0x94000000,
0x00030001,
0x00000000,
0x6d000400,
0x12800021,
0xaa1503e0,
0x94000000,
0x00030002,
0x00000000,
0x52800002,
0x000a0205,
0x92800001,
0x000a0205,
0xaa1503e0,
0x94000000,
0x00030000,
0x52800002,
0x000a0205,
0xf2a00002,
0x000a0205,
0xf2c00002,
0x000a0205,
0xf2e00002,
0x000a0205,
0x12800001,
0xaa1503e0,
0x94000000,
0x00030001,
0x00000000,
0x2d000400,
0x12800021,
0xaa1503e0,
0x94000000,
0x00030002,
0x00000000,
0xaa1503e0,
0x94000000,
0x00030003,
0x00000000,
0x52800002,
0x000a0205,
0xf2a00002,
0x000a0205,
0xf2c00002,
0x000a0205,
0xf2e00002,
0x000a0205,
0x2a1f03e1,
0xaa1503e0,
0x94000000,
0x00030001,
0x00000000,
0xf9000001,
0x12800021,
0xaa1503e0,
0x94000000,
0x00030002,
0x00000000,
0x52800002,
0x000a0205,
0xf2a00002,
0x000a0205,
0xf2c00002,
0x000a0205,
0xf2e00002,
0x000a0205,
0x2a1f03e1,
0xaa1503e0,
0x94000000,
0x00030001,
0x00000000,
0xf9000001,
0x12800021,
0xaa1503e0,
0x94000000,
0x00030002,
0x00000000,
0xaa1503e0,
0x94000000,
0x00030004,
0x00000000,
0x384003e1,
0x000f0000,
0x00000000,
0x38c003e1,
0x000f0000,
0x00000000,
0x38408681,
0x00000000,
0x38c08681,
0x00000000,
0xaa1503e0,
0x94000000,
0x00030005,
0x00000000,
0x784003e1,
0x000f0000,
0x00000000,
0x78c003e1,
0x000f0000,
0x00000000,
0x78408681,
0x00000000,
0x78c08681,
0x00000000,
0xaa1503e0,
0x94000000,
0x00030005,
0x00000000,
0xaa1503e0,
0x94000000,
0x00030005,
0x00000000,
0xaa1503e0,
0x94000000,
0x00030006,
0x00000000,
0xaa1503e0,
0x94000000,
0x00030007,
0x00000000,
0x52800002,
0x000a0205,
0x52800001,
0x000a0205,
0xaa1503e0,
0x94000000,
0x00030008,
0x00000000,
0x52800002,
0x000a0205,
0x92800021,
0xaa1503e0,
0x94000000,
0x00030000,
0x52800003,
0x000a0205,
0xf2a00003,
0x000a0205,
0xf2c00003,
0x000a0205,
0xf2e00003,
0x000a0205,
0x92800002,
0x92800021,
0xaa1503e0,
0x94000000,
0x00030009,
0x00000000,
0x52800002,
0x000a0205,
0x92800021,
0xaa1503e0,
0x94000000,
0x00030000,
0x52800003,
0x000a0205,
0xf2a00003,
0x000a0205,
0xf2c00003,
0x000a0205,
0xf2e00003,
0x000a0205,
0x92800002,
0x92800021,
0xaa1503e0,
0x94000000,
0x0003000a,
0xaa0003f4,
0x12800041,
0xaa1503e0,
0x94000000,
0x0003000b,
0x00000000,
0x2d410e82,
0x00000000,
0x6d410e82,
0x00000000,
0xbd400a82,
0x00000000,
0xfd400a82,
0x00000000,
0x2d400680,
0x00000000,
0x6d400680,
0x00000000,
0xbd400280,
0x00000000,
0xfd400280,
0x00000000,
0xf84003e0,
0x000f0000,
0xaa1403e1,
0x52800002,
0x000a0205,
0xf2a00002,
0x000a0205,
0xf2c00002,
0x000a0205,
0xf2e00002,
0x000a0205,
0x52800009,
0x000a0205,
0xf2a00009,
0x000a0205,
0xf2c00009,
0x000a0205,
0xf2e00009,
0x000a0205,
0xd63f0120,
0x00000000,
0xa9400680,
0x00000000,
0xf9400280,
0x00000000,
0x52800002,
0x000a0205,
0x92800021,
0xaa1503e0,
0x94000000,
0x00030000,
0x52800003,
0x000a0205,
0xf2a00003,
0x000a0205,
0xf2c00003,
0x000a0205,
0xf2e00003,
0x000a0205,
0x92800002,
0x92800021,
0xaa1503e0,
0x94000000,
0x0003000c,
0x00000000,
0x12800021,
0xaa1503e0,
0x94000000,
0x0003000b,
0x00000000,
0x12800001,
0x00000000,
0xaa1503e0,
0x94000000,
0x0003000d,
0x00000000,
0xaa1503e0,
0x94000000,
0x0003000e,
0x00000000,
0x12800001,
0x00000000,
0xaa1503e0,
0x94000000,
0x0003000f,
0x00000000,
0xaa1503e0,
0x94000000,
0x00030010,
0x00000000,
0x12800001,
0xaa1503e0,
0x94000000,
0x00030011,
0x00000000,
0x12800001,
0xaa1503e0,
0x94000000,
0x00030012,
0x00000000,
0x9e660014,
0x12800041,
0xaa1503e0,
0x94000000,
0x0003000b,
0x9e670280,
0x00000000,
0x12800001,
0xaa1503e0,
0x94000000,
0x00030013,
0x00000000,
0x9e660014,
0x12800041,
0xaa1503e0,
0x94000000,
0x0003000b,
0x9e670280,
0x00000000,
0x12800001,
0xaa1503e0,
0x94000000,
0x00030014,
0x00000000,
0x12800001,
0xaa1503e0,
0x94000000,
0x00030015,
0x00000000,
0xaa1503e0,
0x12800041,
0x9e660014,
0x9e660035,
0x94000000,
0x0003000b,
0x9e670280,
0x9e6702a1,
0x00000000,
0xaa0003f4,
0x12800041,
0xaa1503e0,
0x94000000,
0x0003000b,
0xaa1403e0,
0x00000000,
0xa9407bfd,
0xa94157f4,
0x910003ff,
0x000c0000,
0xd65f03c0,
0x00000000,
0x6d000fe2,
0x000a8cef,
0x00000000,
0xfc0003e2,
0x000f0000,
0x00000000,
0x6d0007e0,
0x000a8cef,
0x00000000,
0xfc0003e0,
0x000f0000,
0x00000000,
0x2c810680,
0x2c810e82,
0x00000000,
0x6c810680,
0x6c810e82,
0x00000000,
0x2c810680,
0xbc008682,
0x00000000,
0x6c810680,
0xfc008682,
0x00000000,
0x2c810680,
0x00000000,
0x6c810680,
0x00000000,
0xbc008680,
0x00000000,
0xfc008680,
0x00000000,
0xb80003e0,
0x000f0000,
0x00000000,
0xb8000680,
0x000a812c,
0x00000000,
0xf80003e0,
0x000f0000,
0x00000000,
0xf8000680,
0x000a812c,
0x00000000,
0xa90007e0,
0x000a8cef,
0x00000000,
0xa8810680,
0x00000000,
0xd100c3ff,
0xf80143f5,
0xa90153f3,
0xa9007bfd,
0x910003fd,
0xaa0003f5,
0x00000000,
0x52800009,
0x000a0205,
0xf2a00009,
0x000a0205,
0xcb2963ff,
0x00000000,
0xd10003ff,
0x000c0000,
0x00000000,
0x94000000,
0x00030016,
0x7100001f,
0x000c0000,
0x5400000a,
0x00050801,
0x52800001,
0x000a0205,
0xf2a00001,
0x000a0205,
0xf2c00001,
0x000a0205,
0xf2e00001,
0x000a0205,
0xaa1503e0,
0x94000000,
0x00030017,
0x0006000b,
0xaa0003f3,
0x91000400,
0xb34003e0,
0xcb206fff,
0x00000000,
0x910203f4,
0x00000000,
0x52800003,
0x000a0205,
0xf2a00003,
0x000a0205,
0xf2c00003,
0x000a0205,
0xf2e00003,
0x000a0205,
0x52800002,
0x000a0205,
0x72a00002,
0x000a0205,
0x52800001,
0x000a0205,
0x00000000,
0xaa1503e0,
0x94000000,
0x00030009,
0x00000000,
0xaa1503e0,
0x94000000,
0x00030018,
0x00000000,
0xaa1503e0,
0x94000000,
0x0003000c,
0x00000000,
0xaa1503e0,
0x94000000,
0x0003000a,
0x00000000,
0x2d410c02,
0x00000000,
0x6d410c02,
0x00000000,
0xbd400802,
0x00000000,
0xfd400802,
0x00000000,
0x2d400400,
0x00000000,
0x6d400400,
0x00000000,
0xbd400000,
0x00000000,
0xfd400000,
0x00000000,
0xaa0003e1,
0x00000000,
0x52800000,
0x000a0205,
0xf2a00000,
0x000a0205,
0xcb0003a0,
0x00000000,
0xd10003a0,
0x000c0000,
0x00000000,
0x52800002,
0x000a0205,
0x52800009,
0x000a0205,
0xf2a00009,
0x000a0205,
0xf2c00009,
0x000a0205,
0xf2e00009,
0x000a0205,
0xd63f0120,
0x00000000,
0xb9400000,
0x00000000,
0xf9400000,
0x00000000,
0xa9400400,
0x00000000,
0x52800001,
0x000a0205,
0x00000000,
0xaa1503e0,
0x94000000,
0x0003000d,
0x6b1f001f,
0x1a9f07e0,
0x00000000,
0xaa1503e0,
0x94000000,
0x0003000e,
0x00000000,
0xaa1503e0,
0x94000000,
0x00030010,
0x00000000,
0xaa1503e0,
0x94000000,
0x00030011,
0x00000000,
0xaa1503e0,
0x94000000,
0x00030013,
0x00000000,
0xaa1503e0,
0x94000000,
0x00030012,
0x00000000,
0xaa1503e0,
0x94000000,
0x00030014,
0x00000000,
0xaa1503e0,
0x94000000,
0x00030015,
0x00000000,
0x910003e4,
0x000c0000,
0x52800003,
0x000a0205,
0xaa1303e2,
0x52800001,
0x000a0205,
0xaa1503e0,
0x94000000,
0x00030019,
0x00000000,
0x910003e4,
0x000c0000,
0x52800003,
0x000a0205,
0xaa1303e2,
0x52800001,
0x000a0205,
0xaa1503e0,
0x94000000,
0x0003001a,
0x00000000,
0xf100027f,
0x000c0000,
0x5400000d,
0x00050801,
0xaa1403e5,
0x52800003,
0x000a0205,
0x52800003,
0x000a0205,
0xaa1303e2,
0x52800001,
0x000a0205,
0xaa1503e0,
0x94000000,
0x0003001b,
0x0006000b,
0x00000000,
0x52800002,
0x000a0205,
0xf2a00002,
0x000a0205,
0xf2c00002,
0x000a0205,
0xf2e00002,
0x000a0205,
0x52800001,
0xaa1503e0,
0x94000000,
0x00030001,
0xaa0003e8,
0x00000000,
0xa9471fe6,
0x00000000,
0xa94617e4,
0x00000000,
0xa9450fe2,
0x00000000,
0xa94407e0,
0x00000000,
0x6d431fe6,
0x00000000,
0x6d4217e4,
0x00000000,
0x6d410fe2,
0x00000000,
0x6d4007e0,
0x00000000,
0x910203ff,
0x00000000,
0x52800009,
0x000a0205,
0xf2a00009,
0x000a0205,
0xf2c00009,
0x000a0205,
0xf2e00009,
0x000a0205,
0xd63f0120,
0x00000000,
0xaa0003f4,
0x52800002,
0x000a0205,
0xf2a00002,
0x000a0205,
0xf2c00002,
0x000a0205,
0xf2e00002,
0x000a0205,
0x52800001,
0x000a0205,
0x72a00001,
0x000a0205,
0xaa1503e0,
0x94000000,
0x00030001,
0xf9000014,
0x52800020,
0x910003bf,
0xa9407bfd,
0xa94153f3,
0xf94013f5,
0x9100c3ff,
0xd65f03c0,
0x00000000,
0xaa0003e1,
0xaa1503e0,
0x94000000,
0x00030003,
0x52800020,
0x910003bf,
0xa9407bfd,
0xa94153f3,
0xf94013f5,
0x9100c3ff,
0xd65f03c0,
0x00000000,
0xaa0003f4,
0x52800002,
0x000a0205,
0xf2a00002,
0x000a0205,
0xf2c00002,
0x000a0205,
0xf2e00002,
0x000a0205,
0x2a1f03e1,
0xaa1503e0,
0x94000000,
0x00030001,
0xf9000014,
0x52800020,
0x910003bf,
0xa9407bfd,
0xa94153f3,
0xf94013f5,
0x9100c3ff,
0xd65f03c0,
0x00000000,
0x2a1f03e0,
0x910003bf,
0xa9407bfd,
0xa94153f3,
0xf94013f5,
0x9100c3ff,
0xd65f03c0,
0x00000000,
0x2a0003e1,
0xaa1503e0,
0x94000000,
0x00030004,
0x52800020,
0x910003bf,
0xa9407bfd,
0xa94153f3,
0xf94013f5,
0x9100c3ff,
0xd65f03c0,
0x00000000,
0x2a0003e1,
0x00000000,
0x53001c21,
0xaa1503e0,
0x94000000,
0x0003001c,
0x00000000,
0x13001c21,
0xaa1503e0,
0x94000000,
0x00030005,
0x00000000,
0x52800020,
0x910003bf,
0xa9407bfd,
0xa94153f3,
0xf94013f5,
0x9100c3ff,
0xd65f03c0,
0x00000000,
0x2a0003e1,
0x00000000,
0x53003c21,
0xaa1503e0,
0x94000000,
0x0003001c,
0x00000000,
0x13003c21,
0xaa1503e0,
0x94000000,
0x00030005,
0x00000000,
0x52800020,
0x910003bf,
0xa9407bfd,
0xa94153f3,
0xf94013f5,
0x9100c3ff,
0xd65f03c0,
0x00000000,
0x2a0003e1,
0x00000000,
0xaa1503e0,
0x94000000,
0x0003001c,
0x00000000,
0xaa1503e0,
0x94000000,
0x00030005,
0x00000000,
0x52800020,
0x910003bf,
0xa9407bfd,
0xa94153f3,
0xf94013f5,
0x9100c3ff,
0xd65f03c0,
0x00000000,
0xaa1503e0,
0x94000000,
0x00030006,
0x52800020,
0x910003bf,
0xa9407bfd,
0xa94153f3,
0xf94013f5,
0x9100c3ff,
0xd65f03c0,
0x00000000,
0xaa1503e0,
0x94000000,
0x00030007,
0x52800020,
0x910003bf,
0xa9407bfd,
0xa94153f3,
0xf94013f5,
0x9100c3ff,
0xd65f03c0,
0x00000000,
0x1e260000,
0x1e260021,
0xaa018000,
0xaa0003f4,
0x52800002,
0x000a0205,
0xf2a00002,
0x000a0205,
0xf2c00002,
0x000a0205,
0xf2e00002,
0x000a0205,
0x52800001,
0x000a0205,
0x72a00001,
0x000a0205,
0xaa1503e0,
0x94000000,
0x00030001,
0xf9000014,
0x52800020,
0x910003bf,
0xa9407bfd,
0xa94153f3,
0xf94013f5,
0x9100c3ff,
0xd65f03c0,
0x00000000,
0x9e660013,
0x9e660034,
0x52800002,
0x000a0205,
0xf2a00002,
0x000a0205,
0xf2c00002,
0x000a0205,
0xf2e00002,
0x000a0205,
0x52800001,
0x000a0205,
0x72a00001,
0x000a0205,
0xaa1503e0,
0x94000000,
0x00030001,
0xa9005013,
0x52800020,
0x910003bf,
0xa9407bfd,
0xa94153f3,
0xf94013f5,
0x9100c3ff,
0xd65f03c0,
0x00000000,
0x52800001,
0x000a0205,
0x72a00001,
0x000a0205,
0xaa1503e0,
0x94000000,
0x0003001d,
0x12800021,
0xaa1503e0,
0x94000000,
0x0003001e,
0x00000000,
0x52800020,
0x910003bf,
0xa9407bfd,
0xa94153f3,
0xf94013f5,
0x9100c3ff,
0xd65f03c0,
0x00000000,
0x52800000,
0x910003bf,
0xa9407bfd,
0xa94153f3,
0xf94013f5,
0x9100c3ff,
0xd65f03c0,
0x00000000,
0x6dbf0fe2,
0x00000000,
0xfc1f0fe2,
0x00000000,
0x9e660033,
0x00000000,
0x9e660014,
0x00000000,
0xaa0103f3,
0x00000000,
0xaa0003f4,
0x00000000,
0x52800002,
0x000a0205,
0xf2a00002,
0x000a0205,
0xf2c00002,
0x000a0205,
0xf2e00002,
0x000a0205,
0x52800001,
0x000a0205,
0x72a00001,
0x000a0205,
0xaa1503e0,
0x94000000,
0x00030001,
0x00000000,
0x6cc107e0,
0x00000000,
0x2d010400,
0x00000000,
0x6d010400,
0x00000000,
0xfc4107e0,
0x00000000,
0xbd000800,
0x00000000,
0xfd000800,
0x00000000,
0x9e670260,
0xbd000400,
0x00000000,
0xf9000413,
0x00000000,
0x9e670280,
0xbd000000,
0x00000000,
0xf9000014,
0x00000000,
0xf9000413,
0x00000000,
0xf9000014,
0x00000000,
0x52800020,
0x910003bf,
0xa9407bfd,
0xa94153f3,
0xf94013f5,
0x9100c3ff,
0xd65f03c0,
0x00000000
};

static const char *const globnames[] = {
  (const char *)0
};
static const char *const extnames[] = {
  "rawgeti",
  "push_cdata",
  "lua_remove",
  "lua_pushinteger",
  "lua_pushboolean",
  "push_int",
  "push_float",
  "lua_pushnumber",
  "lua_call",
  "check_typed_pointer",
  "check_struct",
  "lua_settop",
  "check_enum",
  "check_uint32",
  "check_int32",
  "check_uint64",
  "check_int64",
  "check_uintptr",
  "check_float",
  "check_double",
  "check_complex_double",
  "check_complex_float",
  "lua_gettop",
  "luaL_error",
  "check_typed_cfunction",
  "unpack_varargs_float",
  "unpack_varargs_int",
  "unpack_varargs_stack_skip",
  "push_uint",
  "lua_pushvalue",
  "lua_setuservalue",
  (const char *)0
};

#define JUMP_SIZE 16

//in aarch64 the pc is indicated the current 
#define MIN_BRANCH ((INT32_MIN) >> 6)
#define MAX_BRANCH ((INT32_MAX) >> 6)
//arm64 pc has no offset so comparing with next instruction is -4 
#define BRANCH_OFF -4
#define ROUND_UP(x, align) (((int) (x) + (align - 1)) & ~(align - 1))

static void compile_extern_jump(struct jit* jit, lua_State* L, cfunction func, uint8_t* code)
{
    /* The jump code is the function pointer followed by a stub to call the
     * function pointer. The stub exists so we can jump to functions with an
     * offset greater than 128MB.
     *
     * Note we have to manually set this up since there are commands buffered
     * in the jit state.
     */
	 
	 //l: ptr
	 *(cfunction*) code = func;
	 // ldr x9,#-8
	  *(uint32_t*) &code[8] = 0x58FFFFC9;
	 //br x9
	 *(uint32_t*) &code[12] = 0xD61F0120;
	
}

//| ldr reg, >5
//| b >6
//|5:
//|.long64 val
//|6:



void compile_globals(struct jit* jit, lua_State* L)
{
    (void) jit;
}
typedef struct reg_info{
	uint8_t ints;
	uint8_t floats;
	uint16_t ex;
} reg_info;

static ALWAYS_INLINE bool is_float_type(int t){
    return t==FLOAT_TYPE||t==DOUBLE_TYPE;
}

static int hfa_size(lua_State* L,int idx, const struct ctype* ct,int* isfloat){
	struct ctype* mt;
	int type,ele_count,i,ct_usr;
	lua_getuservalue(L,idx);
	ct_usr=lua_absindex(L,-1);
    lua_rawgeti(L,ct_usr,1);
    mt=(struct ctype*)lua_touserdata(L,-1);
    if(mt==NULL||(mt->pointers&&!mt->is_array)||mt->is_reference||!is_float_type(mt->type)){
        lua_pop(L,2);
        return 0;
    }
	type=mt->type;
    ele_count=(int)(ct->base_size/mt->base_size);
    if(ele_count>4||ct->base_size%mt->base_size){
        lua_pop(L,2);
        return 0;
    }
	lua_pop(L,1);
    for (i = 2;i<=4; ++i) {
        lua_rawgeti(L,ct_usr,i);
		if(lua_isnil(L,-1)){//case have array member;
            lua_pop(L,1);
            break;
        }
        mt=(struct ctype*)lua_touserdata(L,-1);
        if(mt->type!=type||(mt->pointers&&!mt->is_array)||mt->is_reference||!is_float_type(mt->type)){
            lua_pop(L,2);
            return 0;
        }
        lua_pop(L,1);
    }
	if(isfloat){
		*isfloat=mt->type==FLOAT_TYPE;
	}
	lua_pop(L,1);
    return ele_count;
}

static reg_info caculate_regs(lua_State* L,int ct_usr,int nargs){
    int i;reg_info regs;
    const struct ctype* mt;
    for (i = 1,regs.ints=0,regs.floats=0; i <= nargs&&(regs.floats<8||regs.ints<8); ++i){
		lua_rawgeti(L, ct_usr, i);
        mt = (const struct ctype*) lua_touserdata(L, -1);
		if (mt->pointers || mt->is_reference) {
			if(regs.ints<8)regs.ints++;
		}else{
			switch(mt->type){
				case COMPLEX_DOUBLE_TYPE:
				case COMPLEX_FLOAT_TYPE:
					if(regs.floats<7)
						regs.floats+=2;
					else if(regs.floats==7)
						regs.floats=8;
					break;
				case FLOAT_TYPE:
				case DOUBLE_TYPE:
					if(regs.floats<8) ++regs.floats;
					break;
				case STRUCT_TYPE:{
                    int hfasize=hfa_size(L,-1,mt,NULL);
                    if(hfasize>0){
						regs.floats+=hfasize;
						if(regs.floats>8)
							regs.floats=8;
						break;
                    }
                }
				case UNION_TYPE:{
					int size=mt->base_size;
					if(size>16){//passed by address
						if(regs.ints<8)++regs.ints;
						break;
					}
					if(mt->is_empty){
						break; //ignored empty struct
					}
					size=(size+7)>>3;
					if(regs.ints+size<=8) regs.ints+=size;
					break;
				}
				default:
					if(regs.ints<8)++regs.ints;//no need to check type support here
			}
		}
		lua_pop(L,1);
	}
	
	return regs;
}

// arm store/load range for immediate value is only -256-255
static ALWAYS_INLINE void load_int(struct jit* Dst,reg_info* regs){
	if(regs->ints<8)
		dasm_put(Dst, 0, 0x60+(regs->ints++<<3));
	else
		dasm_put(Dst, 3, (regs->ex++,8));
}

static void load_float(struct jit* Dst,reg_info* regs,int isfloat,int exSize){
	if(regs->floats+exSize<8){
		switch(exSize){
			case 3:
				dasm_put(Dst, 6, 0x30+(regs->floats<<3));
				goto l_dual;
			case 2:
			    dasm_put(Dst, 9, 0x30+(regs->floats<<3));
			case 1:
			    l_dual:
				dasm_put(Dst, 12, 0x20+(regs->floats<<3));
				break;
			case 0:
			    dasm_put(Dst, 15, 0x20+(regs->floats<<3));
				break;
		}
		regs->floats+=exSize+1;
		
	}else{
		regs->floats=8;
		regs->ex+=exSize+1;
		switch(exSize){
			case 3:
				if(isfloat){
					dasm_put(Dst, 18);
				}else{
					dasm_put(Dst, 21);
				}
			     break;
			case 2:
				if(isfloat){ //12 bytes rounded to 16
					dasm_put(Dst, 24);
					break;
				}else {
					dasm_put(Dst, 27);
				}
			     break;
			case 1:
				if(isfloat){
					dasm_put(Dst, 30);
				}else{
					dasm_put(Dst, 32);
				}
				break;
			case 0:
				if(isfloat){
					dasm_put(Dst, 34);
				}else {
					dasm_put(Dst, 36);
				}
				break;
		}
	
	}
}

cfunction compile_callback(lua_State* L, int fidx, int ct_usr, const struct ctype* ct)
{
    struct jit* Dst = get_jit(L);;
    int i, nargs, num_upvals, ref,ret_by_addr;
    const struct ctype* mt;

    int top = lua_gettop(L);

    ct_usr = lua_absindex(L, ct_usr);
    fidx = lua_absindex(L, fidx);
    nargs = (int) lua_rawlen(L, ct_usr);

    dasm_setup(Dst, build_actionlist);

    lua_newtable(L);
    lua_pushvalue(L, -1);
    ref = luaL_ref(L, LUA_REGISTRYINDEX);
    num_upvals = 0;

    if (ct->has_var_arg) {
        luaL_error(L, "can't create callbacks with varargs");
    }
	
	lua_rawgeti(L, ct_usr, 0);
    mt = (const struct ctype*) lua_touserdata(L, -1);
	ret_by_addr=!mt->pointers && !mt->is_reference &&(mt->type==STRUCT_TYPE||mt->type==UNION_TYPE)
				&& mt->base_size>16&& !(mt->type==STRUCT_TYPE&&hfa_size(L,-1,mt,NULL)!=0);
	if(ret_by_addr){
		dasm_put(Dst, 38);
	}
	lua_pop(L,1);	
	reg_info regs=caculate_regs(L,ct_usr,nargs);
	
	if(regs.ints||regs.floats){
		dasm_put(Dst, 40);
	}else{
		dasm_put(Dst, 42);
	}
	//8 integer reigsters and 8 floating registers
	switch(regs.ints){
		case 8:
		case 7:
			dasm_put(Dst, 44);
		case 6:
		case 5:
			dasm_put(Dst, 46);
		case 4:
		case 3:
			dasm_put(Dst, 48);
		case 2:
		case 1:
			dasm_put(Dst, 50);
	}	

	switch(regs.floats){
		case 8:
		case 7:
			dasm_put(Dst, 52);
		case 6:
		case 5:
			dasm_put(Dst, 54);
		case 4:
		case 3:
			dasm_put(Dst, 56);
		case 2:
		case 1:
			dasm_put(Dst, 58);
	} 
	dasm_put(Dst, 60);
	
	if(regs.ints==8||regs.floats==8){ // may be overflowed if it's full
		dasm_put(Dst, 63, 0xa0+ret_by_addr*0x10);
	}
	
    /* get the lua function */
    lua_pushvalue(L, fidx);
    lua_rawseti(L, -2, ++num_upvals);
	
	dasm_put(Dst, 66, (unsigned short)(L), (((unsigned int)(L))>>16), (unsigned short)((unsigned long)(L)>>32), (unsigned short)((unsigned long)(L)>>48), (unsigned short)(ref), (((unsigned int)(ref))>>16), (unsigned short)(LUA_REGISTRYINDEX), (((unsigned int)(LUA_REGISTRYINDEX))>>16));
	
    dasm_put(Dst, 86, num_upvals);
	

    for (i = 1,regs.ints=0,regs.floats=0; i <= nargs; ++i) {
        lua_rawgeti(L, ct_usr, i);
        mt = (const struct ctype*) lua_touserdata(L, -1);

        if (mt->pointers || mt->is_reference) {
            lua_getuservalue(L, -1);
            lua_rawseti(L, -3, ++num_upvals); /* usr value */
            lua_rawseti(L, -2, ++num_upvals); /* mt */
			
            dasm_put(Dst, 93, num_upvals-1, i, (unsigned short)(mt), (((unsigned int)(mt))>>16), (unsigned short)((unsigned long)(mt)>>32), (unsigned short)((unsigned long)(mt)>>48));
			load_int(Dst,&regs);
            dasm_put(Dst, 113);

        } else {
            switch (mt->type) {
			case STRUCT_TYPE:
			case UNION_TYPE:{
				int isfloat,hfasize=0;
				if(mt->type!=UNION_TYPE){
					hfasize=hfa_size(L,-1,mt,&isfloat);
				}
				lua_getuservalue(L, -1);
				lua_rawseti(L, -3, ++num_upvals); /* usr value */
				lua_rawseti(L, -2, ++num_upvals); /* mt */
                dasm_put(Dst, 119, num_upvals-1, i, (unsigned short)(mt), (((unsigned int)(mt))>>16), (unsigned short)((unsigned long)(mt)>>32), (unsigned short)((unsigned long)(mt)>>48));
				
				if(hfasize){
					load_float(Dst,&regs,isfloat,hfasize-1);
					switch(hfasize){
						case 4:
							if(isfloat){
								dasm_put(Dst, 139);
							}else{
								dasm_put(Dst, 141);
							}
							goto hfa2;
						case 3:
							if(isfloat){
								dasm_put(Dst, 143);
							}else{
								dasm_put(Dst, 145);
							}
						case 2:
            			hfa2:
							if(isfloat){
								dasm_put(Dst, 147);
							}else{
								dasm_put(Dst, 149);
							}
							break;
						case 1:
							if(isfloat){
								dasm_put(Dst, 151);
							}else{
								dasm_put(Dst, 153);
							}
							break;
         							
					}
				}else if(!mt->is_empty){
					size_t size=mt->base_size;
					if(size>16){
						load_int(Dst,&regs);
						dasm_put(Dst, 155, (unsigned short)(mt->base_size), (((unsigned int)(mt->base_size))>>16), (unsigned short)((unsigned long)(mt->base_size)>>32), (unsigned short)((unsigned long)(mt->base_size)>>48), (unsigned short)(memcpy), (((unsigned int)(memcpy))>>16), (unsigned short)((unsigned long)(memcpy)>>32), (unsigned short)((unsigned long)(memcpy)>>48));
					}else{
						size=(size+7)>>3;
						if(mt->align_mask>8){
							if(regs.ints&1) regs.ints++;
							else if(regs.ex&1) regs.ex++;
						}
						if(regs.ints+size<=8){
							if(size>1){
								dasm_put(Dst, 173, 0x60+(regs.ints<<3));
							}else{
								dasm_put(Dst, 177, 0x60+(regs.ints<<3));
							}
							regs.ints+=size;
						}else{
							regs.ints=8;
							if(size>1){
								dasm_put(Dst, 181);
							}else{
								dasm_put(Dst, 184);
							}
							
						} 
					}
					
					
				}
				
                dasm_put(Dst, 187);
			    break;
			}
			case COMPLEX_DOUBLE_TYPE:
				lua_getuservalue(L, -1);
				lua_rawseti(L, -3, ++num_upvals); /* usr value */
				lua_rawseti(L, -2, ++num_upvals); /* mt */
				
                dasm_put(Dst, 192, num_upvals-1, i, (unsigned short)(mt), (((unsigned int)(mt))>>16), (unsigned short)((unsigned long)(mt)>>32), (unsigned short)((unsigned long)(mt)>>48));
				load_float(Dst,&regs,0,1);
                dasm_put(Dst, 212);
				
				break;
			case COMPLEX_FLOAT_TYPE:
				lua_getuservalue(L, -1);
				lua_rawseti(L, -3, ++num_upvals); /* usr value */
				lua_rawseti(L, -2, ++num_upvals); /* mt */
				
                dasm_put(Dst, 218, num_upvals-1, i, (unsigned short)(mt), (((unsigned int)(mt))>>16), (unsigned short)((unsigned long)(mt)>>32), (unsigned short)((unsigned long)(mt)>>48));
				load_float(Dst,&regs,1,1);
                dasm_put(Dst, 238);
				
				break;
            case INT64_TYPE:
			    #if LUA_VERSION_NUM>=503
                lua_pop(L, 1);
				load_int(Dst,&regs);
                dasm_put(Dst, 244);
				
				#else
                lua_rawseti(L, -2, ++num_upvals); /* mt */
				
                dasm_put(Dst, 248, (unsigned short)(mt), (((unsigned int)(mt))>>16), (unsigned short)((unsigned long)(mt)>>32), (unsigned short)((unsigned long)(mt)>>48));
                load_int(Dst,&regs);
                dasm_put(Dst, 261);
				
				#endif
                break;

            case INTPTR_TYPE:
                lua_rawseti(L, -2, ++num_upvals); /* mt */
				
                dasm_put(Dst, 267, (unsigned short)(mt), (((unsigned int)(mt))>>16), (unsigned short)((unsigned long)(mt)>>32), (unsigned short)((unsigned long)(mt)>>48));
                load_int(Dst,&regs);
                dasm_put(Dst, 280);
				
                break;

            case BOOL_TYPE:
                lua_pop(L, 1);
				
				load_int(Dst,&regs);
				dasm_put(Dst, 286);
				
                break;

            case INT8_TYPE:// need to narrow for caller doesn't do it
				lua_pop(L, 1);
				if(regs.ints<8){
					if (mt->is_unsigned) {
						dasm_put(Dst, 290, 0x60+(regs.ints++<<3));
					} else {
						dasm_put(Dst, 293, 0x60+(regs.ints++<<3));
					}
				}else {
					if (mt->is_unsigned) {
						dasm_put(Dst, 296);
					} else {
						dasm_put(Dst, 298);
					}
				}
				dasm_put(Dst, 300);
				break;
			
            case INT16_TYPE:// need to narrow for caller doesn't do it
				lua_pop(L, 1);
				if(regs.ints<8){
					if (mt->is_unsigned) {
						dasm_put(Dst, 304, 0x60+(regs.ints++<<3));
					} else {
						dasm_put(Dst, 307, 0x60+(regs.ints++<<3));
					}
				}else {
					if (mt->is_unsigned) {
						dasm_put(Dst, 310);
					} else {
						dasm_put(Dst, 312);
					}
				}
				dasm_put(Dst, 314);
				break;
				
            case ENUM_TYPE:
            case INT32_TYPE:
                lua_pop(L, 1);
				load_int(Dst,&regs);
				
                dasm_put(Dst, 318);
                break;

            case FLOAT_TYPE:
                lua_pop(L, 1);
				load_float(Dst,&regs,1,0);
                dasm_put(Dst, 322);
                break;

            case DOUBLE_TYPE:
                lua_pop(L, 1);
				load_float(Dst,&regs,0,0);
                dasm_put(Dst, 326);
                break;
            default:
                luaL_error(L, "NYI: callback arg type");
            }
        }
    }

    lua_rawgeti(L, ct_usr, 0);
    mt = (const struct ctype*) lua_touserdata(L, -1);

    dasm_put(Dst, 330, ((mt->pointers || mt->is_reference || mt->type != VOID_TYPE) ? 1 : 0), nargs);
    

	
    if (mt->pointers || mt->is_reference) {
        lua_getuservalue(L, -1);
        lua_rawseti(L, -3, ++num_upvals); /* usr value */
        lua_rawseti(L, -2, ++num_upvals); /* mt */

        dasm_put(Dst, 338, num_upvals-1, (unsigned short)(mt), (((unsigned int)(mt))>>16), (unsigned short)((unsigned long)(mt)>>32), (unsigned short)((unsigned long)(mt)>>48));
        goto single_no_pop;
    } else {
        switch (mt->type) {
		case STRUCT_TYPE:
		case UNION_TYPE:{
			int hfasize=0,isfloat;
			if(mt->type!=UNION_TYPE){
				hfasize=hfa_size(L,-1,mt,&isfloat);
			}
			lua_getuservalue(L, -1);
			lua_rawseti(L, -3, ++num_upvals); /* usr value */
			lua_rawseti(L, -2, ++num_upvals); /* mt */
            dasm_put(Dst, 358, num_upvals-1, (unsigned short)(mt), (((unsigned int)(mt))>>16), (unsigned short)((unsigned long)(mt)>>32), (unsigned short)((unsigned long)(mt)>>48));
			if(hfasize){
				switch(hfasize){
					case 4:
					    if(isfloat){
							dasm_put(Dst, 383);
						}else{
							dasm_put(Dst, 385);
						}
						goto ld_hfa;
					case 3:
						if(isfloat){
							dasm_put(Dst, 387);
						}else{
							dasm_put(Dst, 389);
						}
					case 2:
						ld_hfa:
					    if(isfloat){
							dasm_put(Dst, 391);
						}else{
							dasm_put(Dst, 393);
						}
						break;
					case 1:
						if(isfloat){
							dasm_put(Dst, 395);
						}else{
							dasm_put(Dst, 397);
						}
					    break;
				}
			}else{
				if(mt->base_size>16){
					dasm_put(Dst, 399, 0x20+((regs.ints||regs.floats)?0x80:0), (unsigned short)(mt->base_size), (((unsigned int)(mt->base_size))>>16), (unsigned short)((unsigned long)(mt->base_size)>>32), (unsigned short)((unsigned long)(mt->base_size)>>48), (unsigned short)(memcpy), (((unsigned int)(memcpy))>>16), (unsigned short)((unsigned long)(memcpy)>>32), (unsigned short)((unsigned long)(memcpy)>>48));
				}else{
					if(mt->base_size>8){
						dasm_put(Dst, 420);
					}else{
						dasm_put(Dst, 422);
					}
				}
			}
			break;
		}
        case ENUM_TYPE:
            lua_getuservalue(L, -1);
            lua_rawseti(L, -3, ++num_upvals); /* usr value */
            lua_rawseti(L, -2, ++num_upvals); /* mt */

            dasm_put(Dst, 424, num_upvals-1, (unsigned short)(mt), (((unsigned int)(mt))>>16), (unsigned short)((unsigned long)(mt)>>32), (unsigned short)((unsigned long)(mt)>>48));

            goto single_no_pop;

        case VOID_TYPE:
            dasm_put(Dst, 444);
            lua_pop(L, 1);
            break;

        case BOOL_TYPE:
        case INT8_TYPE:
        case INT16_TYPE:
        case INT32_TYPE: //caller's responsiblity to narrow 
		    dasm_put(Dst, 449);
            if (mt->is_unsigned) {
                dasm_put(Dst, 451);
            } else {
                dasm_put(Dst, 455);
            }
			
            goto single;

        case INT64_TYPE:
            dasm_put(Dst, 459);
            if (mt->is_unsigned) {
                dasm_put(Dst, 461);
            } else {
                dasm_put(Dst, 465);
            }
			
			goto single;

        case INTPTR_TYPE:
            dasm_put(Dst, 469);
            goto single;

        case FLOAT_TYPE:
            dasm_put(Dst, 474);
			
            dasm_put(Dst, 479);
            lua_pop(L, 1);
			break;
        case DOUBLE_TYPE:
            dasm_put(Dst, 486);
			
            dasm_put(Dst, 491);
			
            lua_pop(L, 1);
			break;
			
		case COMPLEX_DOUBLE_TYPE:

			dasm_put(Dst, 498);
		    
			goto complex_ret;
		case COMPLEX_FLOAT_TYPE:
			dasm_put(Dst, 503);
			
		complex_ret:	
			dasm_put(Dst, 508);

            lua_pop(L, 1);			
			break;
        single:
            lua_pop(L, 1);
		single_no_pop:	
            dasm_put(Dst, 517);
            break;

        
        default:
            luaL_error(L, "NYI: callback return type");
        }
    }
	
	dasm_put(Dst, 524,  (0x20 +ret_by_addr*0x10+ ((regs.floats!=0)||(regs.ints!=0)) * 0x80));
	
    lua_pop(L, 1); /* upval table - already in registry */
    assert(lua_gettop(L) == top);

    {
        void* p;
        struct ctype ft;
        cfunction func;

        func = compile(Dst, L, NULL, ref);

        ft = *ct;
        ft.is_jitted = 1;
        p = push_cdata(L, ct_usr, &ft);
        *(cfunction*) p = func;

        assert(lua_gettop(L) == top + 1);

        return func;
    }
}

//arm64 argument can only be in stack or registers. An argument can't be splited between stack and register.
static  void store_float(struct jit* Dst,reg_info* regs,int isfloat,int ex){
	if(regs->floats+ex<8){
		
		switch(ex){
			case 3:
				dasm_put(Dst, 530, 0x10+(regs->floats<<3));
			     goto sd_dual;
			case 2:
			    dasm_put(Dst, 533, 0x10+(regs->floats<<3));
			case 1:
			    sd_dual:
				dasm_put(Dst, 536, (regs->floats<<3));
				break;
			case 0:
				dasm_put(Dst, 539, (regs->floats<<3));
				break;
		}
		regs->floats+=1+ex;
	}else {
		regs->floats=8;
		switch(ex){
			case 3:
			    if(isfloat){
					dasm_put(Dst, 542);
				}else{
					dasm_put(Dst, 545);
				}				
			    break;
			case 2:
			    if(isfloat){
					dasm_put(Dst, 548);
				}else{
					dasm_put(Dst, 551);
				}
				break;
			case 1:
			    if(isfloat){
					dasm_put(Dst, 554);
				}else{
					dasm_put(Dst, 556);
				}
				break;
			case 0:
				if(isfloat){
					dasm_put(Dst, 558);
				}else {
					dasm_put(Dst, 560);
				}
				break;
		}
		//complex float is packed as one double on stack
		regs->ex+=ex+1;
	} 
}

static void store_int(struct jit* Dst,reg_info* regs,int intSize){
	switch(intSize){
		case 1:
			if(regs->ints<8)
				dasm_put(Dst, 562, 0x40+(regs->ints++<<3));
			else
				dasm_put(Dst, 565, (regs->ex++,8));
            break;
		case 2:
			if(regs->ints<8)
				dasm_put(Dst, 568, 0x40+(regs->ints++<<3));
			else
				dasm_put(Dst, 571, (regs->ex++,8));
			break;
		case 3:
		case 4:
			if(regs->ints<7){
				dasm_put(Dst, 574, 0x40+(regs->ints<<3));
				regs->ints+=2;
			}
			else{
				if(regs->ints==7)
					regs->ints=8;
				dasm_put(Dst, 577);
				regs->ex+=2;
			}
				
			break;
	}
}

static int caculate_stack(lua_State* L,int ct_usr,int nargs){
    int i;reg_info regs={0,0,0};
    const struct ctype* mt;int stack=0,extra=0;
    for (i = 1; i <= nargs; ++i){
		lua_rawgeti(L, ct_usr, i);
        mt = (const struct ctype*) lua_touserdata(L, -1);
		if (mt->pointers || mt->is_reference) {
			if(regs.ints<8)regs.ints++;
			else stack++;
		}else{
			switch(mt->type){
				case COMPLEX_DOUBLE_TYPE:
				case COMPLEX_FLOAT_TYPE:
					if(regs.floats<7)
						regs.floats+=2;
					else if(regs.floats==7)
						regs.floats=8;
					else stack+=mt->base_size>>3;
					break;
				case FLOAT_TYPE:
				case DOUBLE_TYPE:
					if(regs.floats<8) ++regs.floats;
					else stack++;
					break;
				case STRUCT_TYPE:{
					int isfloat;
                    int hfasize=hfa_size(L,-1,mt,&isfloat);
                    if(hfasize>0){
						if(regs.floats+hfasize<=8)
							regs.floats +=hfasize;
						else {
						    regs.floats=8;
							stack+=(hfasize*(2-isfloat)+1)>>1;	
						}
						break;
                    }
                }
				case UNION_TYPE:{
					int size=mt->base_size;
					size=(size+7)>>3;
					if(size>2){//passed by address
						if(regs.ints<8)++regs.ints;
						else stack++;
					    extra+=size;//extra copy stack;
						break;
					}
					if(mt->is_empty){
						break; //ignored empty struct
					}
					if(mt->align_mask>8){
						if(regs.ints&1) regs.ints++;
						else if(stack&1) stack++;
					}
					if(regs.ints+size<=8) regs.ints+=size;
					else{
						regs.ints=8;
						stack+=size;
					} 
					
					break;
				}
				default:
					if(regs.ints<8)++regs.ints;//no need to check type support here
					else stack++;
			}
		}
		lua_pop(L,1);
	}
	
	return (regs.ints||regs.floats)?((stack+extra+17/*16 for regs, 1 for align*/)>>1)<<4:0;//2 eightbytes align
}


void compile_function(lua_State* L, cfunction func, int ct_usr, const struct ctype* ct)
{
    struct jit* Dst = get_jit(L);;
    int i, nargs, num_upvals,ret_by_addr;
    const struct ctype* mt;
	int stack_size,struct_offset;
    void* p;

    int top = lua_gettop(L);

    ct_usr = lua_absindex(L, ct_usr);
    nargs = (int) lua_rawlen(L, ct_usr);

    p = push_cdata(L, ct_usr, ct);
    *(cfunction*) p = func;
    num_upvals = 1;

    dasm_setup(Dst, build_actionlist);

    reg_info regs={0,0};
	
	dasm_put(Dst, 579);
	
    /* reserve enough stack space for all of the arguments. */
	stack_size=caculate_stack(L,ct_usr,nargs);
	struct_offset=0;
	if(stack_size>0){
		if(stack_size>=1<<12){
			dasm_put(Dst, 586, (unsigned short)(stack_size), (((unsigned int)(stack_size))>>16));
		}
		else{
			dasm_put(Dst, 592, stack_size);
		}
		 if (ct->has_var_arg) {
			dasm_put(Dst, 595, nargs, (unsigned short)("too few arguments"), (((unsigned int)("too few arguments"))>>16), (unsigned short)((unsigned long)("too few arguments")>>32), (unsigned short)((unsigned long)("too few arguments")>>48));
		}
		dasm_put(Dst, 618);
    }
	
    for (i = 1,regs.ints=0,regs.floats=0; i <= nargs; i++) {
        lua_rawgeti(L, ct_usr, i);
        mt = (const struct ctype*) lua_touserdata(L, -1);
		
        if (mt->pointers || mt->is_reference || mt->type == FUNCTION_PTR_TYPE || mt->type == ENUM_TYPE||mt->type==STRUCT_TYPE||mt->type==UNION_TYPE) {
            lua_getuservalue(L, -1);
            num_upvals += 2;

			
			dasm_put(Dst, 620, (unsigned short)(mt), (((unsigned int)(mt))>>16), (unsigned short)((unsigned long)(mt)>>32), (unsigned short)((unsigned long)(mt)>>48), (unsigned short)(lua_upvalueindex(num_upvals)), (((unsigned int)(lua_upvalueindex(num_upvals)))>>16), i);
			
            if (mt->pointers || mt->is_reference) {
                dasm_put(Dst, 635);
            } else{
				switch (mt->type) {
					case FUNCTION_PTR_TYPE: {
						dasm_put(Dst, 639);
						break;
					}
					case ENUM_TYPE:{
						dasm_put(Dst, 643);
						break;
					} 
					case STRUCT_TYPE:
					case UNION_TYPE:{
						if(mt->is_empty) continue;
		
						int isfloat;
						int hfasize=hfa_size(L,-2,mt,&isfloat);
						dasm_put(Dst, 647);
                        if(hfasize>0){
							switch(hfasize){
								case 4:
									if(isfloat){
										dasm_put(Dst, 651);
									}else{
										dasm_put(Dst, 653);
									}
									goto ld_hfa;
								case 3:
									if(isfloat){
										dasm_put(Dst, 655);
									}else{
										dasm_put(Dst, 657);
									}
								case 2:
									ld_hfa:
									if(isfloat){
										dasm_put(Dst, 659);
									}else{
										dasm_put(Dst, 661);
									}
									break;
								case 1:
									if(isfloat){
										dasm_put(Dst, 663);
									}else{
										dasm_put(Dst, 665);
									}
									break;
									
							}
							store_float(Dst,&regs,isfloat,hfasize-1);
							continue;
						}
						if(mt->base_size>16){
							dasm_put(Dst, 667);
							struct_offset+=(mt->base_size+7)&(~7);
							if(struct_offset>=1<<12){
								dasm_put(Dst, 669, (unsigned short)(struct_offset), (((unsigned int)(struct_offset))>>16));
							}
							else{
								dasm_put(Dst, 675, struct_offset);
							}
							store_int(Dst,&regs,2);
							dasm_put(Dst, 678, mt->base_size, (unsigned short)(memcpy), (((unsigned int)(memcpy))>>16), (unsigned short)((unsigned long)(memcpy)>>32), (unsigned short)((unsigned long)(memcpy)>>48));
						}else{
							if(mt->align_mask>8){//==15
								if(regs.ints&1) regs.ints++;
								else if(regs.ex&1) regs.ex++;
							}
							int intSize=(mt->base_size+3)>>2;
							switch(intSize){
								case 1:
									dasm_put(Dst, 690);
									break;
								case 2:
									dasm_put(Dst, 692);
									break;
								case 3:
								case 4:
								    dasm_put(Dst, 694);
									break;
							}
							store_int(Dst,&regs,intSize);
						}
						continue;
					}	
				}
			}
			goto longstore;

        } else {
            lua_pop(L, 1);
            dasm_put(Dst, 696, i);

            switch (mt->type) {
            case BOOL_TYPE:
                dasm_put(Dst, 699);
                goto intstore;
				
            case INT8_TYPE:
            case INT16_TYPE: //arm64 requires callee to narrow the type
            case INT32_TYPE:
                
                dasm_put(Dst, 705);
                
 				goto intstore;

            case INT64_TYPE:
               
                dasm_put(Dst, 709);
               
              	goto longstore;
				
            case INTPTR_TYPE:
                dasm_put(Dst, 713);
                
                goto longstore;

            case DOUBLE_TYPE:
                dasm_put(Dst, 717);
                store_float(Dst,&regs,0,0);
                break;

            case FLOAT_TYPE:
                dasm_put(Dst, 721);
                store_float(Dst,&regs,1,0);
                break;
			case COMPLEX_DOUBLE_TYPE:
                dasm_put(Dst, 725);
				store_float(Dst,&regs,0,1);
                break;

            case COMPLEX_FLOAT_TYPE:
                dasm_put(Dst, 729);
				store_float(Dst,&regs,1,1);
                break;
				
			intstore:
				store_int(Dst,&regs,1);
                break;
			longstore:
				store_int(Dst,&regs,2);
                break;
				
            default:
                luaL_error(L, "NYI: call arg type");
            }
        }
    }

    if (ct->has_var_arg) {
		if(regs.floats<8){
			dasm_put(Dst, 733, regs.floats<<3, (8-regs.floats), nargs+1);
		}
		if(regs.ints<8){
			dasm_put(Dst, 744, 0x40+(regs.ints<<3), (8-regs.ints), nargs+1);
		}
		dasm_put(Dst, 755, (nargs>8?nargs:8), (8-regs.floats), (8-regs.ints), nargs+1);
		regs.floats=regs.ints=8;
    }
	
	lua_rawgeti(L, ct_usr, 0);
    mt = (const struct ctype*) lua_touserdata(L, -1);
	ret_by_addr=!mt->pointers && !mt->is_reference &&(mt->type==STRUCT_TYPE||mt->type==UNION_TYPE)
				&& mt->base_size>16&& !(mt->type==STRUCT_TYPE&&hfa_size(L,-1,mt,NULL)!=0);
	if(ret_by_addr){
		dasm_put(Dst, 772, (unsigned short)(mt), (((unsigned int)(mt))>>16), (unsigned short)((unsigned long)(mt)>>32), (unsigned short)((unsigned long)(mt)>>48));
	}
	
	//pop all args in registers
	switch(regs.ints){
		case 8:
		case 7:
            dasm_put(Dst, 786);
		case 6:
		case 5:
            dasm_put(Dst, 788);
		case 4:
		case 3:
            dasm_put(Dst, 790);
		case 2:
		case 1:
			dasm_put(Dst, 792);
    }
	
	switch(regs.floats){
		case 8:
		case 7:
            dasm_put(Dst, 794);
		case 6:
		case 5:
            dasm_put(Dst, 796);
		case 4:
		case 3:
            dasm_put(Dst, 798);
		case 2:
		case 1:
			dasm_put(Dst, 800);
    }
	if(regs.ints==8|| regs.floats==8){// fix stack case registers is full
		dasm_put(Dst, 802);
	}
	
    dasm_put(Dst, 804, (unsigned short)(func), (((unsigned int)(func))>>16), (unsigned short)((unsigned long)(func)>>32), (unsigned short)((unsigned long)(func)>>48));


    if (mt->pointers || mt->is_reference || mt->type==FUNCTION_PTR_TYPE) {
        lua_getuservalue(L, -1);
        num_upvals += 2;
        dasm_put(Dst, 814, (unsigned short)(mt), (((unsigned int)(mt))>>16), (unsigned short)((unsigned long)(mt)>>32), (unsigned short)((unsigned long)(mt)>>48), (unsigned short)(lua_upvalueindex(num_upvals)), (((unsigned int)(lua_upvalueindex(num_upvals)))>>16));

    } else {
        switch (mt->type) {
        case INT64_TYPE:
		#if LUA_VERSION_NUM>=503
			 lua_pop(L, 1);
            dasm_put(Dst, 839);
            break;
		#endif

        case INTPTR_TYPE:
            num_upvals++;
            dasm_put(Dst, 851, (unsigned short)(mt), (((unsigned int)(mt))>>16), (unsigned short)((unsigned long)(mt)>>32), (unsigned short)((unsigned long)(mt)>>48));
			break;
        case VOID_TYPE:
            lua_pop(L, 1);
            dasm_put(Dst, 873);
            break;

        case BOOL_TYPE:
            lua_pop(L, 1);
            dasm_put(Dst, 881);
            break;

        case INT8_TYPE:// we need to narrow the value before return
			lua_pop(L, 1);
            dasm_put(Dst, 893);
            if (mt->is_unsigned) {
                dasm_put(Dst, 895);
            } else {
                dasm_put(Dst, 900);
            }
            dasm_put(Dst, 905);
			break;
        case INT16_TYPE:// we need to narrow the value before return
			lua_pop(L, 1);
            dasm_put(Dst, 913);
            if (mt->is_unsigned) {
                dasm_put(Dst, 915);
            } else {
                dasm_put(Dst, 920);
            }
            dasm_put(Dst, 925);
			break;
        case INT32_TYPE:
        case ENUM_TYPE:
            lua_pop(L, 1);
            dasm_put(Dst, 933);
            if (mt->is_unsigned) {
                dasm_put(Dst, 935);
            } else {
                dasm_put(Dst, 939);
            }
            dasm_put(Dst, 943);
            break;

        case FLOAT_TYPE:
            lua_pop(L, 1);
            dasm_put(Dst, 951);
            break;

        case DOUBLE_TYPE:
            lua_pop(L, 1);
            dasm_put(Dst, 962);
            break;
		case COMPLEX_FLOAT_TYPE:
            lua_getuservalue(L, -1);
            num_upvals+=2;
            dasm_put(Dst, 973, (unsigned short)(mt), (((unsigned int)(mt))>>16), (unsigned short)((unsigned long)(mt)>>32), (unsigned short)((unsigned long)(mt)>>48), (unsigned short)(lua_upvalueindex(num_upvals)), (((unsigned int)(lua_upvalueindex(num_upvals)))>>16));
            break;

        case COMPLEX_DOUBLE_TYPE:
            lua_getuservalue(L, -1);
            num_upvals+=2;
            dasm_put(Dst, 1001, (unsigned short)(mt), (((unsigned int)(mt))>>16), (unsigned short)((unsigned long)(mt)>>32), (unsigned short)((unsigned long)(mt)>>48), (unsigned short)(lua_upvalueindex(num_upvals)), (((unsigned int)(lua_upvalueindex(num_upvals)))>>16));
            break;
		case STRUCT_TYPE:
		case UNION_TYPE:
			lua_getuservalue(L, -1);
            num_upvals+=2;
		    if(ret_by_addr){
				if(!lua_isnil(L,-1)){
					dasm_put(Dst, 1027, (unsigned short)(lua_upvalueindex(num_upvals)), (((unsigned int)(lua_upvalueindex(num_upvals)))>>16));
				}
				dasm_put(Dst, 1039);
			}else if(mt->is_empty){
				dasm_put(Dst, 1047);
			}else{
				int isfloat;
				int hfasize=hfa_size(L,-2,mt,&isfloat);
				if(hfasize){
					switch(hfasize){
						case 4:
							dasm_put(Dst, 1055);
							goto hfs_dual;
						case 3:
							dasm_put(Dst, 1057);
						case 2:
							hfs_dual:
							dasm_put(Dst, 1059);
						case 1:
							dasm_put(Dst, 1061);
							break;
					}
				}else{
					if(mt->base_size>8){
						dasm_put(Dst, 1063);
					}
					dasm_put(Dst, 1065);
					
				} 
				dasm_put(Dst, 1067, (unsigned short)(mt), (((unsigned int)(mt))>>16), (unsigned short)((unsigned long)(mt)>>32), (unsigned short)((unsigned long)(mt)>>48), (unsigned short)(lua_upvalueindex(num_upvals)), (((unsigned int)(lua_upvalueindex(num_upvals)))>>16));
				if(hfasize){
					switch(hfasize){
						case 4:
							dasm_put(Dst, 1083);
							if(isfloat){
								dasm_put(Dst, 1085);
							}else{
								dasm_put(Dst, 1087);
							} 
							goto hfl_dual;
						case 3:
							dasm_put(Dst, 1089);
							if(isfloat){
								dasm_put(Dst, 1091);
							}else{
								dasm_put(Dst, 1093);
							}
						case 2:
							hfl_dual:
							if(isfloat){
								dasm_put(Dst, 1095);
							}else{
								dasm_put(Dst, 1098);
							}
						case 1:
							if(isfloat){
								dasm_put(Dst, 1100);
							}else{
								dasm_put(Dst, 1103);
							}
							break;
					}
				}else{
					if(mt->base_size>8){
						dasm_put(Dst, 1105);
					}
					dasm_put(Dst, 1107);
					
				} 
				dasm_put(Dst, 1109);
			}
			break;	
		
        default:
            luaL_error(L, "NYI: call return type");
        }
    }

    assert(lua_gettop(L) == top + num_upvals);
	{
        cfunction f = compile(Dst, L, NULL, LUA_NOREF);
        /* add a callback as an upval so that the jitted code gets cleaned up when
         * the function gets gc'd */
        push_callback(L, f, func);
        lua_pushcclosure(L, (lua_CFunction) f, num_upvals+1);
    }
}

