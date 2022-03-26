#include "cli/common.hpp"
#include "cryptography/common.hpp"
using namespace cli;

// hybrid cipher test
int main() {
    // generating RSA keys for both parties, normally these keys are given
    RSA::PrivateKey private1 = RSA::GeneratePrivateKey();
    RSA::PublicKey public1 = RSA::GeneratePublicKey(private1);

    RSA::PrivateKey private2 = RSA::GeneratePrivateKey();
    RSA::PublicKey public2 = RSA::GeneratePublicKey(private2);

    RSA::PrivateKey private3 = RSA::GeneratePrivateKey();
    RSA::PublicKey public3 = RSA::GeneratePublicKey(private3);

    // the first party wants to sent this
    std::string plainText1 = "Szia Uram!";

    print::info(plainText1);

    // first party generates an AES key and an InitVector
    AES::Key aes1 = AES::GenerateKey();
    AES::InitVector iv = AES::GenerateInitVector();

    // first party encrypts the AES key with the second party's public key
    std::string cipherKey = RSA::EncryptKey(
        aes1,
        public2
    );

    // first party encrypts the plain text with the AES key
    std::string cipherText1 = AES::Encrypt(
        plainText1,
        aes1,
        iv
    );

    // first party signs the message with its private key
    std::string signature = RSA::SignKey(
        aes1,
        // uncomment private3 to simulate a malicious third person
        private1
        //private3
    );

    // first party cracks the cipher text into 3 parts
    std::vector<std::string> sendable = CrackString(
        cipherText1,
        3
    );

    // first party sends the encrypted AES key, the number of cracks, a crack, 
    // the signature and the plain InitVector to 3 different nodes

    // uncomment for data corruption test
    //sendable[0][0] = 'a';

    // all 3 cracks arrive to the second party on different paths

    // second party assembles the cipher text from the 3 cracks
    std::string cipherText2 = AssembleString(sendable);

    // second party decrypts AES key with its private key
    AES::Key aes2 = RSA::DecryptKey(
        cipherKey,
        private2
    );

    bool dataIntegrity, signatureValidity;

    // second party verifies the signature's validity, which proves 
    // whether the first party actually owns the private key
    signatureValidity = RSA::VerifyKey(
        signature,
        aes2,
        public2
    );

    // second party decrypts the cipher text using the AES key
    std::string plainText2 = AES::Decrypt(
        cipherText2,
        aes2,
        iv,
        dataIntegrity
    );

    if (!dataIntegrity) {
        print::error("data integrity violation");
    }

    if (!signatureValidity) {
        print::error("invalid signature");
    }

    // if everything went fine there are no error messages and you see the initial plain text
    print::info(plainText2);
}