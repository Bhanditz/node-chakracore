//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
#include "RuntimeDebugPch.h"

#if ENABLE_TTD

namespace TTD
{
    namespace UtilSupport
    {
        TTAutoString::TTAutoString()
            : m_allocSize(-1), m_contents(nullptr), m_optFormatBuff(nullptr)
        {
            ;
        }

        TTAutoString::TTAutoString(const char16* str)
            : m_allocSize(-1), m_contents(nullptr), m_optFormatBuff(nullptr)
        {
            size_t clen = wcslen(str) + 1;

            this->m_contents = TT_HEAP_ALLOC_ARRAY_ZERO(char16, clen);
            this->m_allocSize = (int32)clen;

            js_memcpy_s(this->m_contents, clen * sizeof(char16), str, clen * sizeof(char16));
        }

        TTAutoString::TTAutoString(const TTAutoString& str)
            : m_allocSize(-1), m_contents(nullptr), m_optFormatBuff(nullptr)
        {
            this->Append(str.m_contents);
        }

        TTAutoString& TTAutoString::operator=(const TTAutoString& str)
        {
            if(this != &str)
            {
                this->Clear();

                this->Append(str.GetStrValue());
            }

            return *this;
        }

        TTAutoString::~TTAutoString()
        {
            this->Clear();
        }

        void TTAutoString::Clear()
        {
            if(this->m_contents != nullptr)
            {
                TT_HEAP_FREE_ARRAY(char16, this->m_contents, (size_t)this->m_allocSize);
                this->m_allocSize = -1;
                this->m_contents = nullptr;
            }

            if(this->m_optFormatBuff != nullptr)
            {
                TT_HEAP_FREE_ARRAY(char16, this->m_optFormatBuff, 64);
                this->m_optFormatBuff = nullptr;
            }
        }

        bool TTAutoString::IsNullString() const
        {
            return this->m_contents == nullptr;
        }

        void TTAutoString::Append(const char16* str, int32 start, int32 end)
        {
            if(this->m_contents == nullptr && str == nullptr)
            {
                return;
            }

            size_t origsize = (this->m_contents != nullptr ? wcslen(this->m_contents) : 0);
            int32 strsize = -1;
            if(start == 0 && end == INT32_MAX)
            {
                strsize = (str != nullptr ? (int32)wcslen(str) : 0);
            }
            else
            {
                strsize = (end - start) + 1;
            }

            size_t nsize = origsize + strsize + 1;
            char16* nbuff = TT_HEAP_ALLOC_ARRAY_ZERO(char16, nsize);

            if(this->m_contents != nullptr)
            {
                js_memcpy_s(nbuff, nsize * sizeof(char16), this->m_contents, origsize * sizeof(char16));

                TT_HEAP_FREE_ARRAY(char16, this->m_contents, origsize + 1);
                this->m_allocSize = -1;
                this->m_contents = nullptr;
            }

            if(str != nullptr)
            {
                int32 curr = (int32)origsize;
                for(int32 i = start; i <= end && str[i] != '\0'; ++i)
                {
                    nbuff[curr] = str[i];
                    curr++;
                }
                nbuff[curr] = _u('\0');
            }

            this->m_contents = nbuff;
            this->m_allocSize = (int32)nsize;
        }

        void TTAutoString::Append(const TTAutoString& str, int32 start, int32 end)
        {
            this->Append(str.GetStrValue(), start, end);
        }

        void TTAutoString::Append(uint64 val)
        {
            if(this->m_optFormatBuff == nullptr)
            {
                this->m_optFormatBuff = TT_HEAP_ALLOC_ARRAY_ZERO(char16, 64);
            }

            swprintf_s(this->m_optFormatBuff, 32, _u("%I64u"), val); //64 char16s is 32 words

            this->Append(this->m_optFormatBuff);
        }

        void TTAutoString::Append(LPCUTF8 strBegin, LPCUTF8 strEnd)
        {
            int32 strCount = (int32)((strEnd - strBegin) + 1);
            char16* buff = TT_HEAP_ALLOC_ARRAY_ZERO(char16, (size_t)strCount);

            LPCUTF8 curr = strBegin;
            int32 i = 0;
            while(curr != strEnd)
            {
                buff[i] = (char16)*curr;
                i++;
                curr++;
            }
            AssertMsg(i + 1 == strCount, "Our indexing is off.");

            buff[i] = _u('\0');
            this->Append(buff);

            TT_HEAP_FREE_ARRAY(char16, buff, (size_t)strCount);
        }

        int32 TTAutoString::GetLength() const
        {
            AssertMsg(!this->IsNullString(), "That doesn't make sense.");

            return (int32)wcslen(this->m_contents);
        }

        char16 TTAutoString::GetCharAt(int32 pos) const
        {
            AssertMsg(!this->IsNullString(), "That doesn't make sense.");
            AssertMsg(0 <= pos && pos < this->GetLength(), "Not in valid range.");

            return this->m_contents[pos];
        }

        const char16* TTAutoString::GetStrValue() const
        {
            return this->m_contents;
        }
    }

    //////////////////

    void LoadValuesForHashTables(uint32 targetSize, uint32* powerOf2, uint32* closePrime, uint32* midPrime)
    {
        TTD_TABLE_FACTORLOAD_BASE(128, 127, 61)

            TTD_TABLE_FACTORLOAD(256, 251, 127)
            TTD_TABLE_FACTORLOAD(512, 511, 251)
            TTD_TABLE_FACTORLOAD(1024, 1021, 511)
            TTD_TABLE_FACTORLOAD(2048, 2039, 1021)
            TTD_TABLE_FACTORLOAD(4096, 4093, 2039)
            TTD_TABLE_FACTORLOAD(8192, 8191, 4093)
            TTD_TABLE_FACTORLOAD(16384, 16381, 8191)
            TTD_TABLE_FACTORLOAD(32768, 32749, 16381)
            TTD_TABLE_FACTORLOAD(65536, 65521, 32749)
            TTD_TABLE_FACTORLOAD(131072, 131071, 65521)
            TTD_TABLE_FACTORLOAD(262144, 262139, 131071)
            TTD_TABLE_FACTORLOAD(524288, 524287, 262139)
            TTD_TABLE_FACTORLOAD(1048576, 1048573, 524287)
            TTD_TABLE_FACTORLOAD(2097152, 2097143, 1048573)
            TTD_TABLE_FACTORLOAD(4194304, 4194301, 2097143)
            TTD_TABLE_FACTORLOAD(8388608, 8388593, 4194301)

        TTD_TABLE_FACTORLOAD_FINAL(16777216, 16777213, 8388593)
    }

    //////////////////

    void InitializeAsNullPtrTTString(TTString& str)
    {
        str.Length = 0;
        str.Contents = nullptr;
    }

    bool IsNullPtrTTString(const TTString& str)
    {
        return str.Contents == nullptr;
    }

#if ENABLE_TTD_INTERNAL_DIAGNOSTICS
    bool TTStringEQForDiagnostics(const TTString& str1, const TTString& str2)
    {
        if(IsNullPtrTTString(str1) || IsNullPtrTTString(str2))
        {
            return IsNullPtrTTString(str1) && IsNullPtrTTString(str2);
        }

        if(str1.Length != str2.Length)
        {
            return false;
        }

        for(uint32 i = 0; i < str1.Length; ++i)
        {
            if(str1.Contents[i] != str2.Contents[i])
            {
                return false;
            }
        }

        return true;
    }
#endif

    void InitializeAsNullPtrTTStringUtf8(TTStringUtf8& utf8str)
    {
        utf8str.ByteLength = 0;
        utf8str.Contents = nullptr;
    }

    bool IsNullPtrTTStringUtf8(const TTStringUtf8& utf8str)
    {
        return utf8str.Contents == nullptr;
    }

#if ENABLE_TTD_INTERNAL_DIAGNOSTICS
    //This is for diagnostic purposes only
    bool TTStringEQForDiagnosticsUtf8(const TTStringUtf8& utf8str1, const TTStringUtf8& utf8str2)
    {
        if(IsNullPtrTTStringUtf8(utf8str1) || IsNullPtrTTStringUtf8(utf8str2))
        {
            return IsNullPtrTTStringUtf8(utf8str1) && IsNullPtrTTStringUtf8(utf8str2);
        }

        if(utf8str1.ByteLength != utf8str2.ByteLength)
        {
            return false;
        }

        for(uint32 i = 0; i < utf8str1.ByteLength; ++i)
        {
            if(utf8str1.Contents[i] != utf8str2.Contents[i])
            {
                return false;
            }
        }

        return true;
    }
#endif

    //////////////////

    MarkTable::MarkTable()
        : m_capcity(TTD_MARK_TABLE_INIT_SIZE), m_h2Prime(TTD_MARK_TABLE_INIT_H2PRIME), m_count(0), m_iterPos(0)
    {
        this->m_addrArray = TT_HEAP_ALLOC_ARRAY_ZERO(uint64, this->m_capcity);
        this->m_markArray = TT_HEAP_ALLOC_ARRAY_ZERO(MarkTableTag, this->m_capcity);

        memset(this->m_handlerCounts, 0, ((uint32)MarkTableTag::KindTagCount) * sizeof(uint32));
    }

    MarkTable::~MarkTable()
    {
        TT_HEAP_FREE_ARRAY(uint64, this->m_addrArray, this->m_capcity);
        TT_HEAP_FREE_ARRAY(MarkTableTag, this->m_markArray, this->m_capcity);
    }

    void MarkTable::Clear()
    {
        if(this->m_capcity == TTD_MARK_TABLE_INIT_SIZE)
        {
            memset(this->m_addrArray, 0, TTD_MARK_TABLE_INIT_SIZE * sizeof(uint64));
            memset(this->m_markArray, 0, TTD_MARK_TABLE_INIT_SIZE * sizeof(MarkTableTag));
        }
        else
        {
            TT_HEAP_FREE_ARRAY(uint64, this->m_addrArray, this->m_capcity);
            TT_HEAP_FREE_ARRAY(MarkTableTag, this->m_markArray, this->m_capcity);

            this->m_capcity = TTD_MARK_TABLE_INIT_SIZE;
            this->m_h2Prime = TTD_MARK_TABLE_INIT_H2PRIME;
            this->m_addrArray = TT_HEAP_ALLOC_ARRAY_ZERO(uint64, this->m_capcity);
            this->m_markArray = TT_HEAP_ALLOC_ARRAY_ZERO(MarkTableTag, this->m_capcity);
        }

        this->m_count = 0;
        this->m_iterPos = 0;

        memset(this->m_handlerCounts, 0, ((uint32)MarkTableTag::KindTagCount) * sizeof(uint32));
    }
}

#endif
