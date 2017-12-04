#include "polarcode/errordetection/cmac.h"
#include <polarcode/avxconvenience.h>
#include <openssl/evp.h>
#include <openssl/cmac.h>
#include <cstring>
#include <iostream>
#include <stdexcept>


namespace PolarCode {
    namespace ErrorDetection {


        cmac::cmac(std::vector<unsigned char> initKey, unsigned int bitCount) :
            mBitCount(bitCount), mMacKey(nullptr)
        {
          setKey(initKey);

          for(int i = 0; i < 16; i++){
            printf("%.2X ", mMacKey[i]);
          }
          std::cout << std::endl;
        }

        cmac::~cmac() {
          _mm_free(mMacKey);
        }

        void cmac::setKey(std::vector<unsigned char> key)
        {
          if(key.size() != 16){
            throw std::invalid_argument("MAC key length must be equal to 16bytes!");
          }
          _mm_free(mMacKey);
          mMacKey = static_cast<unsigned char*>(_mm_malloc(16, BYTESPERVECTOR));
          memcpy(mMacKey, &key[0], 16);
        }

        size_t
        cmac::calculate_cmac_len(unsigned char* cmac, const unsigned int cmac_len, const unsigned char* key, const unsigned int key_len, const unsigned char* message, const unsigned int msg_len)
        {
          unsigned char t_cmac[] = { 0x00,0x00,0x00,0x00,
                                     0x00,0x00,0x00,0x00,
                                     0x00,0x00,0x00,0x00,
                                     0x00,0x00,0x00,0x00 };
          calculate_cmac(t_cmac, key, key_len, message, msg_len);
          std::memcpy(cmac, t_cmac, sizeof(unsigned char) * cmac_len);
          return cmac_len;
        }

        size_t
        cmac::calculate_cmac(unsigned char* cmac, const unsigned char* key, const unsigned int key_len, const unsigned char* message, const unsigned int msg_len)
        {
          /*
          // https://tools.ietf.org/html/rfc4493

          // K, M and T from
          // old: http://csrc.nist.gov/publications/nistpubs/800-38B/Updated_CMAC_Examples.pdf
          // new: http://nvlpubs.nist.gov/nistpubs/SpecialPublications/NIST.SP.800-38b.pdf
          // D.1 AES-128

          // https://csrc.nist.gov/CSRC/media/Projects/Cryptographic-Standards-and-Guidelines/documents/examples/AES_CMAC.pdf
          // Following: example 2 is shown!


          // K: 2b7e1516 28aed2a6 abf71588 09cf4f3c
          const unsigned char e_key[] = { 0x2b,0x7e,0x15,0x16,
                                          0x28,0xae,0xd2,0xa6,
                                          0xab,0xf7,0x15,0x88,
                                          0x09,0xcf,0x4f,0x3c};

          // M: 6bc1bee2 2e409f96 e93d7e11 7393172a Mlen: 128
          const unsigned char e_message[] = { 0x6b,0xc1,0xbe,0xe2,
                                              0x2e,0x40,0x9f,0x96,
                                              0xe9,0x3d,0x7e,0x11,
                                              0x73,0x93,0x17,0x2a };

          // expected result T = 070a16b4 6b4d4144 f79bdd9d d04a287c
          const unsigned char e_cmac[] = {0x07, 0x0a, 0x16, 0xb4,
                                          0x6b, 0x4d, 0x41, 0x44,
                                          0xf7, 0x9b, 0xdd, 0x9d,
                                          0xd0, 0x4a, 0x28, 0x7c};


          According to
          https://wiki.openssl.org/index.php/EVP_Signing_and_Verifying
          CMAC calculation via the standard EVP_Sign* interface is only supported in OpenSSL 1.1.0 and up.
          We use OpenSSL 1.0.1f at the moment.

          EVP_PKEY* pkey = NULL;
          EVP_PKEY_CTX* kctx = EVP_PKEY_CTX_new_id(EVP_PKEY_CMAC, NULL);
          EVP_PKEY_keygen_init(kctx);

          EVP_PKEY_CTX_ctrl(kctx, -1, EVP_PKEY_OP_KEYGEN, EVP_PKEY_CTRL_CIPHER, 0, (void*) EVP_aes_128_cbc());
          EVP_PKEY_CTX_ctrl(kctx, -1, EVP_PKEY_OP_KEYGEN, EVP_PKEY_CTRL_SET_MAC_KEY, key_len, (void*) key);
          EVP_PKEY_keygen(kctx, &pkey);
          EVP_PKEY_CTX_free(kctx);


          const EVP_MD* md = EVP_get_digestbyname("MD5");
          EVP_MD_CTX* ctx = EVP_MD_CTX_create();

          EVP_DigestSignInit(ctx, NULL, md, NULL, pkey);

          EVP_DigestSignUpdate(ctx, message, msg_len);

          size_t req = 0;
          EVP_DigestSignFinal(ctx, NULL, &req);
          EVP_DigestSignFinal(ctx, cmac, &req);

          EVP_MD_CTX_destroy(ctx);
          ctx = NULL;

          return req;
          */


          size_t mactlen;
          CMAC_CTX *ctx = CMAC_CTX_new();
          CMAC_Init(ctx, key, key_len, EVP_aes_128_cbc(), NULL);
          CMAC_Update(ctx, message, msg_len);
          CMAC_Final(ctx, cmac, &mactlen);
          CMAC_CTX_free(ctx);

          return mactlen;
        }

        unsigned char cmac::gen(unsigned char *data, int bytes) {
          unsigned char chkSum = 0;
          calculate_cmac_len(&chkSum, 1, mMacKey, 16, data, bytes);
          return chkSum;
        }

        bool cmac::check(void *pData, int bytes) {
          unsigned char *data = reinterpret_cast<unsigned char*>(pData);
          unsigned char toCheck = gen(data, bytes-1);
          return toCheck == data[bytes-1];
        }

        void cmac::generate(void *pData, int bytes) {
          unsigned char *data = reinterpret_cast<unsigned char*>(pData);
          unsigned char chkSum = gen(data, bytes-1);
          data[bytes-1] = chkSum;
        }

        int cmac::multiCheck(void **pData, int nArrays, int nBytes) {
          unsigned char **data = reinterpret_cast<unsigned char**>(pData);

          int firstMatch = -1;
          for(int array = 0; array < nArrays; ++array) {
            if(check(data[array], nBytes)) {
              firstMatch = array;
              break;
            }
          }

          return firstMatch;
        }

    }//namespace ErrorDetection
}//namespace PolarCode
