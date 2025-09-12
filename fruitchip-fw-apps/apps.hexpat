struct Entry {
    u32 data_len;
    u32 attrs;
    u8 data[data_len];
    u32 crc32;
};

struct LookupEntry {
    u32 *offset: u32;
    u32 entry_len;
};

struct Apps {
    u32 magic1;
    u32 magic2;
    u32 version;
    u32 entries_count;
    LookupEntry lookup[entries_count];
    Entry entries[entries_count];
};

Apps apps @ 0x0;
