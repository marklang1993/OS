#include <fstream>
#include <iostream>
#include <sstream>

#include "drivers/hdd.h"

#include "partition.h"

using namespace std;

bool Partition::read
(
    partition_table_entry* pTable,
    UINT32 pos,
    UINT32 count
)
{
    ifstream fImg;
    fImg.open(m_imgName, ios::binary);

    if (fImg.fail())
    {
        cerr<<"Failed to open "<<m_imgName<<endl;
        return false;
    }
    // Reset file cursor
    fImg.seekg(pos, ios::beg);

    UINT32 size = count * PARTITION_TABLE_ENTRY_SIZE;
    BYTE* pBuf = reinterpret_cast<BYTE*>(pTable);
    // Read Partition Table
    for (UINT32 i = 0; i < size; ++i)
    {
        // Check ifstream flags
        if (fImg.fail() || fImg.eof())
        {
            return false;
        }
        // Read 1 byte
        char tmp;
        fImg.get(tmp);
        *(pBuf + i) = static_cast<BYTE>(tmp);
    }

    fImg.close();
    return true;
}

void Partition::print
(
    partition_table_entry* const pTable,
    UINT32* pOffset,
    UINT32 count,
    bool isExtend
)
{
    // Process
    UINT32 i, j;
    j = 0;

    for (i = 0; i < count; ++i)
    {
        partition_table_entry* pEntry = pTable + i;
        // Check null partition
        if (pEntry->type == 0)
        {
            ++j;
            continue;
        }

        // Check extended partition
        if (pEntry->type == PART_TYPE_EXTENDED && !isExtend)
            continue;

        // Output Main Partition Table
        cout<<"Partition Table "<<i + *pOffset<<" - ";
        cout<<"Bootable: "<<
            string((pEntry->is_bootable & PART_BOOTABLE) != 0 ? "Yes" : "No");
        cout<<" Type: 0x"<<hex<<static_cast<UINT32>(pEntry->type);
        cout<<dec;
        cout<<" Base sector: "<<pEntry->base_sector_lba;
        cout<<" Count: "<<pEntry->cnt_sectors;
        cout<<endl;
        ++j;
    }

    *pOffset += j;
}

void Partition::Print()
{
    partition_table_entry pMainTable[COUNT_M_PART_TABLE_ENTRY];
    UINT32 partitionTableOffset = 1;
    // Read Main Partition Table
    if (!read(pMainTable, BASE_PARTITION_TABLE, COUNT_M_PART_TABLE_ENTRY))
        return;

    // Print Main Partition Table
    print(pMainTable,
          &partitionTableOffset,
          COUNT_M_PART_TABLE_ENTRY,
          true
          );

    // Process Logic Partitions
    partition_table_entry pLogicTable[COUNT_L_PART_TABLE_ENTRY];
    for (UINT32 i = 0; i < COUNT_M_PART_TABLE_ENTRY; ++i)
    {
        // Check: current partition is extended partition
        if (pMainTable[i].type != PART_TYPE_EXTENDED)
            continue;

        // Read logic partition table
        UINT32 extendPartBase =
            pMainTable[i].base_sector_lba
            * HDD_BYTES_PER_SECTOR
            + BASE_PARTITION_TABLE;
        if (!read(pLogicTable, extendPartBase, COUNT_L_PART_TABLE_ENTRY))
            return;

        // Print logic partition table
        print(pLogicTable,
              &partitionTableOffset,
              COUNT_L_PART_TABLE_ENTRY,
              false
              );

        // Check next logic partiton table
        while (pLogicTable[1].type == PART_TYPE_EXTENDED)
        {
            // Read next logic partition table
            UINT32 nextPartBase = 
                extendPartBase
                + pLogicTable[1].base_sector_lba
                * HDD_BYTES_PER_SECTOR;
            if (!read(pLogicTable, nextPartBase, COUNT_L_PART_TABLE_ENTRY))
                return;

            // Print logic partition table
            print(pLogicTable,
                  &partitionTableOffset,
                  COUNT_L_PART_TABLE_ENTRY,
                  false
                  );
        }
    }
}


int main(int argc, char* argv[])
{
    Partition partition(string("hdd.img"));
    partition.Print();

    return 0;
}
