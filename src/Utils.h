#pragma once


namespace REL
{
    template <typename T>
    void SafeWriteT(uintptr_t a_addr, const T a_data)
    {
        safe_write(a_addr, &a_data, sizeof(T));
    }
}