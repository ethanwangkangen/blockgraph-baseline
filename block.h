#ifndef BLOCK_H
#define BLOCK_H
#include <iostream>
#include <string>
#include <vector>
#include "transaction.h"

using namespace std;

class Block{

  public:
    static const int HASH_SIZE = 32;
    enum{regBlock=0, mergeBlock=1};

  public:
    // Typedef
    typedef struct block_t{
      double              timestamp;  // time of creation of a block
      int                 size;  // size of block expresed in bytes
      int                 index; // block height
      int                 leader; // node id that minned the block
      int                 parents_count; 
      int                 tx_count;
      int                blockType;   // new
      char                groupId[HASH_SIZE]; // Id of the group that validated the block  //[HASH_SIZE]
      char                hash[HASH_SIZE]; // hash of the current block  /[HASH_SIZE]
    } block_t;


  public:
    //constructor and destructor
    Block();
    Block(string hash, int index, int leader, int blockType, string groupId,  // new
          vector<string> parents, double timestamp,
          vector<Transaction> transactions);
    Block(const Block &b);
    Block(const string &serie);
    ~Block();

  public:
    //Getters, Setters & Operators
    string GetHash (void) const;
    void SetHash (string hash);

    int GetIndex (void);
    void SetIndex (int index);

    int GetLeader (void);
    void SetLeader (int leader);

    string GetGroupId (void);
    void SetGroupId (string groupId);

    vector<string> GetParents (void) const;
    void SetParents (vector<string> parents);

    double GetTimestamp (void);
    void SetTimestamp (double timestamp);

    int GetBlockType (void);
    void SetBlockType (int type);

    vector<Transaction> GetTransactions (void) const;
    void SetTransactions (vector<Transaction> transactions);

    int GetSize (void);
    void SetSize (int size);

    int GetTxsCount();

    bool operator==(const Block &b);
    friend std::ostream& operator<< (std::ostream &out, const Block &block);
    Block& operator= (const Block &block2);


  public:
    /**
    * Checks if the block given in the argument is part of the group
    */
    bool IsPartOfGroup(const string groupId);

    /**
    * Checks if it is the parent of the block given in the argument
    */
    bool IsParent (Block &block);

    /**
    * Checks if it is the child of the block given in the argument
    */
    bool IsChild (Block &block);

    /**
    * Checks if the block is a merge block
    */
    bool IsMergeBlock (); //

    /**
     * check if the block merges branches identified by parents' hashes in
     * parents
     */
    bool IsMergeBlock (vector<string> parents);

    /*
     * Calculate the hash of the block
     */
    string CalculateHash ();

    /**
     * Calculate and return the block size
     */
    int CalculateSize();

    /**
    * Calculate and return the size of transactions in block
    */
    int CalculeTxsSize();

    /**
     * Calculate and return the block header size
     */
    int CalculateHeaderSize();

    /**
     * Serialize the block into a string of bytes.
     */
    string Serialize();

  private:

    string              hash;
    int                 index;
    int                 leader;
    string              groupId;
    vector<string>      parents;
    double              timestamp;
    vector<Transaction> transactions;
    int                 size;
    int                blockType;  // new
};

#endif
