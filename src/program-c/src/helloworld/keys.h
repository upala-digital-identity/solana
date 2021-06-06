#pragma once
#include "/home/boris/.local/share/solana/install/releases/1.6.10/solana-release/bin/sdk/bpf/c/inc/solana_sdk.h"

//ATokenGPvbdGVxr1b2hvZbsiqW5xWH25efTNsLJA8knL
const SolPubkey associated_token_program_id = (SolPubkey){.x={
                                             140, 151,  37, 143,  78,  36, 137, 241,
                                             187,  61,  16,  41,  20, 142,  13, 131,
                                             11,  90,  19, 153, 218, 255,  16, 132,
                                             4, 142, 123, 216, 219, 233, 248,  89 }};

//TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA
SolPubkey spl_program_id = (SolPubkey){.x={
                                  6, 221, 246, 225, 215, 101, 161,
                                  147, 217, 203, 225,  70, 206, 235,
                                  121, 172,  28, 180, 133, 237,  95,
                                  91,  55, 145,  58, 140, 245, 133,
                                  126, 255,   0, 169 }};

// sheettoken
// Hg58dqK5ZQVcgpjFQvYXZAXWNZM7wcPRTNWBib1ZMhR5
SolPubkey spl_token_mint_id = (SolPubkey){.x={
                                   247, 189,  80, 100, 143, 255,   8, 65,
                                   5, 139,  47,  74,  95, 141, 208, 24,
                                   157,   3,  74,  52,  14,  95, 180,  4,
                                   186, 240, 116,  50, 189, 199,  67, 52 }};

//11111111111111111111111111111111
SolPubkey system_program_id = (SolPubkey){.x={
                                   0, 0, 0, 0, 0, 0, 0, 0, 0,
                                   0, 0, 0, 0, 0, 0, 0, 0, 0,
                                   0, 0, 0, 0, 0, 0, 0, 0, 0,
                                   0, 0, 0, 0, 0 }};

//SysvarRent111111111111111111111111111111111
SolPubkey rent_sysvar_id = (SolPubkey){.x={
                                6, 167, 213,  23,  25,  44,  92,  81,
                                33, 140, 201,  76,  61,  74, 241, 127,
                                88, 218, 238,   8, 155, 161, 253,  68,
                                227, 219, 217, 138,   0,   0,   0,   0 }};

//2tYsExCoTx9UKEXSh8aKPXVeax7pKEJfBWGdpJ45E8Sq
//glue cook empty unlock improve exhibit nose pulp high lady shop forest
SolPubkey upala_owner_id = (SolPubkey){.x={
                             28,  16,  86, 229,  52,  33, 227,  14,
                             216,  86,  57, 160, 199, 218, 181, 212,
                             157,  27,  68, 188,  26, 171, 136,  37,
                             15, 190, 129, 100,  73, 139, 112, 190 }};
