// Copyright (c) 2022 Dániel Gergely, Dénes Balogh
// Distributed under the MIT License.

#pragma once
#include "common.hpp"

namespace AES {
    typedef CryptoPP::SecByteBlock Key;
    typedef std::shared_ptr<CryptoPP::byte> InitVector;

    static bool IsValid = true; 

    AES::Key GenerateKey() {
        CryptoPP::AutoSeededRandomPool prng;
        CryptoPP::SecByteBlock key(16);
        prng.GenerateBlock(
            key, 
            key.size()
        );
        return key;
    }
    
    AES::InitVector GenerateInitVector() {
        CryptoPP::AutoSeededRandomPool prng;
        // initialization vector
        std::shared_ptr<CryptoPP::byte> iv(
            new CryptoPP::byte[16], 
            std::default_delete<CryptoPP::byte[]>() 
        );
        prng.GenerateBlock(
            iv.get(), 
            sizeof(iv.get())
        );
        return iv;
    }

    std::string Encrypt(
        const std::string& _plainText,
        const AES::Key& _key,
        const AES::InitVector& _iv
    ) {
        const int tagSize = 12;
        std::string cipherText;
        
        try {
            CryptoPP::GCM<CryptoPP::AES>::Encryption e;
            e.SetKeyWithIV(
                _key, 
                _key.size(), 
                _iv.get(), 
                sizeof(_iv.get()) 
            );

            CryptoPP::StringSource ss1(
                _plainText, 
                true,
                new CryptoPP::AuthenticatedEncryptionFilter( 
                    e,
                    new CryptoPP::StringSink(cipherText),
                    false, 
                    tagSize
                )
            );
        } catch (CryptoPP::Exception& e) {
            print::fatal(e.what());
        }
        return cipherText;
    }

    std::string Decrypt(
        const std::string& _cipherText,
        const AES::Key& _key,
        const AES::InitVector& _iv,
        bool& _isValid = IsValid
    ) {
        const int tagSize = 12;
        std::string plainText;
        try {
            CryptoPP::GCM<CryptoPP::AES>::Decryption d;
            d.SetKeyWithIV( 
                _key, 
                _key.size(), 
                _iv.get(), 
                sizeof(_iv.get()) 
            );

            CryptoPP::AuthenticatedDecryptionFilter df( 
                d,
                new CryptoPP::StringSink(plainText),
                CryptoPP::AuthenticatedDecryptionFilter::DEFAULT_FLAGS, 
                tagSize
            );

            CryptoPP::StringSource ss2( 
                _cipherText, 
                true,
                new CryptoPP::Redirector(df /*, PASS_EVERYTHING */)
            );

            // If the object does not throw, here's the only
            //  opportunity to check the data's integrity
            if (df.GetLastResult() == false) {
                //print::error("AES::Decrypt(): data integrity violation");
                _isValid = false;
                return std::string();
            }
            
            _isValid = true;
            return plainText;
        } catch(CryptoPP::Exception& e) {
            //print::error(e.what());
            _isValid = false;
            return std::string();
        }
    }
}