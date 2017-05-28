#ifndef _PARTITION_H_

#include <string>

#include "part_table.h"

#include "cpp_type.h"

class Partition
{

public:
    Partition(std::string imgName) : m_imgName(imgName){ ; }
    ~Partition() = default;

    void Print();

private:
    std::string m_imgName;

    /*
     * Read partition table
     @ pTable : partition table
     @ pos    : offset of partition table
     @ count  : count of partition table entries
     */
    bool read
    (
        partition_table_entry* pTable,
        UINT32 pos,
        UINT32 count
    );

    /*
     * Print partition table
     @ pTable   : partition table
     @ pOffset  : pointer to current partition entry number
     @ count    : count of partition table enties
     @ isExtend : display extended partition or not
     */
    void print(
        const partition_table_entry* const pTable,
        UINT32* pOffset,
        UINT32 count,
        bool isExtend
    );

};

#endif
