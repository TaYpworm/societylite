#include "SocietyPacket.h"

#include <string>
#include <gcrypt.h>
#include <openssl/sha.h>

using namespace std;

int main() {
    string foo("this is a test");
    string bar("this ");
    string foobar("is a test");
    string service("service");
    unsigned char *hash = new unsigned char[20];

    //gcry_md_hash_buffer(GCRY_MD_SHA1, hash, service.c_str);

    SHA_CTX *c;
    SHA1((unsigned char*)service.c_str(), service.size(), hash);
    //SHA1_Init(c);
    //SHA1_Update(c, service.c_str(), service.size());
    //SHA1_Final(hash, c);
    
    for (int i=0; i<20; i++)
        printf("%.02x", hash[i]);
    printf("\n");

    SocietyPacket *one = new SocietyPacket(foo.size(), 1, 1, hash);
    //one->appendPayload((unsigned char *) foo.c_str(), foo.size());
    one->appendPayload((unsigned char *) bar.c_str(), bar.size());
    one->appendPayload((unsigned char *) foobar.c_str(), foobar.size());
    unsigned char *tmp = one->serialize();
   
    SocietyPacket *two = new SocietyPacket();
    two->deserialize(tmp, one->size());
    
    for (int i=0; i<41; i++)
        printf("%.02x", tmp[i]);
    printf("\n");

    printf("%.02x\t%d\t%.02x\t%.02x\t%d\t%d\n", two->ident, two->sequenceNumber, two->priority, two->moreFragments, two->fragmentOffset, two->payloadSize);
    for (int i=0; i<20; i++)
        printf("%.02x", two->source[i]);
    printf("\n");
    printf("%s\n", two->data);

    return 0;
}
