
/*
 * Copyright : (C) 2023 Phytium Information Technology, Inc.
 * All Rights Reserved.
 *
 * This program is OPEN SOURCE software: you can redistribute it and/or modify it
 * under the terms of the Phytium Public License as published by the Phytium Technology Co.,Ltd,
 * either version 1.0 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the Phytium Public License for more details.
 *
 *
 * FilePath: cryptopp_example.c
 * Date: 2022-02-25 14:53:42
 * LastEditTime: 2022-02-28 17:46:03
 * Description:  This file is for crypto++ example implmentation
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2023/6/2   first release
 */

#include "validate.h"
#include "bench.h"

#include "cryptopp_example.h"

extern "C" int FCryptoPPCrc32Example(void)
{
    return CryptoPP::Test::ValidateCRC32C() ? 0 : -1;
}

extern "C" int FCryptoPPMd5Example(void)
{
    return CryptoPP::Test::ValidateMD5()? 0 : -1;
}

extern "C" int FCryptoPPMd4Example(void)
{
    return CryptoPP::Test::ValidateMD4()? 0 : -1;
}

extern "C" int FCryptoPPAdler32Example(void)
{
    return CryptoPP::Test::ValidateAdler32()? 0 : -1;
}