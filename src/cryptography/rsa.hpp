// Copyright (c) 2022 Dániel Gergely, Dénes Balogh
// Distributed under the MIT License.

#pragma once
#include "common.hpp"

namespace RSA {
	typedef CryptoPP::RSAFunction PublicKey;
	typedef CryptoPP::InvertibleRSAFunction PrivateKey;

	PrivateKey GeneratePrivateKey() {
		const size_t size = 3072;
		CryptoPP::AutoSeededRandomPool rng;
		PrivateKey privateKey;
		privateKey.GenerateRandomWithKeySize(rng, size);
		return privateKey;
	}

	PublicKey GeneratePublicKey(
		const PrivateKey& _privateKey
	) {
		return PublicKey(_privateKey);
	}

	void WriteKeyToFile(
		const PrivateKey& _privateKey
	) {
		CryptoPP::FileSink output("private.key");
		_privateKey.DEREncode(output);

		CryptoPP::Base64Encoder privkeysink(new CryptoPP::FileSink("privatekey.dat"));
		_privateKey.DEREncode(privkeysink);
		privkeysink.MessageEnd();
	}

	void WriteKeyToFile(
		const PublicKey& _publicKey
	) {
		CryptoPP::FileSink output2("public.key");
		_publicKey.DEREncode(output2);

		CryptoPP::Base64Encoder pubkeysink(new CryptoPP::FileSink("publickey.dat"));
		_publicKey.DEREncode(pubkeysink);
		pubkeysink.MessageEnd();
	}

	void ReadKeyFromFile(
		char* _filename,
		RSA::PrivateKey& _privateKey
	) {
		CryptoPP::FileSource input(_filename, true, new CryptoPP::Base64Decoder);
		_privateKey.BERDecode(input);
	}

	void ReadKeyFromFile(
		char* _filename,
		RSA::PublicKey& _publicKey
	) {
		CryptoPP::FileSource input(_filename, true, new CryptoPP::Base64Decoder);
		_publicKey.BERDecode(input);
	}

	std::string Encrypt(
		const std::string& _plainText, 
		const RSA::PublicKey& _publicKey
	) {
		std::string cipherText;
		
		CryptoPP::AutoSeededRandomPool rng;

		CryptoPP::RSAES_OAEP_SHA_Encryptor e(_publicKey);

		CryptoPP::StringSource(
			_plainText, 
			true,
			new CryptoPP::PK_EncryptorFilter(
				rng, 
				e,
				new CryptoPP::Base64Encoder(
					new CryptoPP::StringSink(cipherText)
				)
			)
		);

		return cipherText;
	}

	std::string EncryptKey(
		const AES::Key& _plainText, 
		const RSA::PublicKey& _publicKey
	) {
		std::string cipherText;
		
		CryptoPP::AutoSeededRandomPool rng;

		CryptoPP::RSAES_OAEP_SHA_Encryptor e(_publicKey);

		CryptoPP::StringSource(
			_plainText.data(),
			_plainText.size(), 
			true,
			new CryptoPP::PK_EncryptorFilter(
				rng, 
				e,
				new CryptoPP::Base64Encoder(
					new CryptoPP::StringSink(cipherText)
				)
			)
		);

		return cipherText;
	}

	std::string Decrypt(
		const std::string& _cipherText,
		const RSA::PrivateKey& _privateKey
	) {
		std::string plainText;

		CryptoPP::AutoSeededRandomPool rng;

		CryptoPP::RSAES_OAEP_SHA_Decryptor d(_privateKey);

		CryptoPP::StringSource(
			_cipherText, 
			true,
			new CryptoPP::Base64Decoder(
				new CryptoPP::PK_DecryptorFilter(
					rng, 
					d,
					new CryptoPP::StringSink(plainText)
				)
			)
		);
		return plainText;
	}

	AES::Key DecryptKey(
		const std::string& _cipherText,
		const RSA::PrivateKey& _privateKey
	) {
		AES::Key plainText;
		CryptoPP::AutoSeededRandomPool rng;

		CryptoPP::RSAES_OAEP_SHA_Decryptor d(_privateKey);

		CryptoPP::StringSource(
			_cipherText, 
			true,
			new CryptoPP::Base64Decoder(
				new CryptoPP::PK_DecryptorFilter(
					rng, 
					d,
					new CryptoPP::SecByteBlockSink(plainText)
				)
			)
		);
		return plainText;
	}

	std::string Sign(
		const std::string& _plainText,
		const RSA::PrivateKey& _privateKey
	) {
		CryptoPP::AutoSeededRandomPool rng;
		std::string result;

		CryptoPP::RSASS<
			CryptoPP::PSS,
			CryptoPP::SHA256
		>::Signer signer(_privateKey);

		CryptoPP::StringSource(
			_plainText,
			true,
			new CryptoPP::SignerFilter(
				rng,
				signer,
				new CryptoPP::StringSink(result)
			)
		);

		return result;
	}

	std::string SignKey(
		const AES::Key& _plainText,
		const RSA::PrivateKey& _privateKey
	) {
		CryptoPP::AutoSeededRandomPool rng;
		std::string result;

		CryptoPP::RSASS<
			CryptoPP::PSS,
			CryptoPP::SHA256
		>::Signer signer(_privateKey);

		CryptoPP::StringSource(
			_plainText.data(),
			_plainText.size(),
			true,
			new CryptoPP::SignerFilter(
				rng,
				signer,
				new CryptoPP::StringSink(result)
			)
		);

		return result;
	}

	bool Verify(
		const std::string& _signature,
		const std::string& _plainText,
		const RSA::PublicKey& _publicKey
	) {
		try {
			CryptoPP::RSASS<
				CryptoPP::PSS,
				CryptoPP::SHA256
			>::Verifier verifier(_publicKey);

			CryptoPP::StringSource(
				_plainText + _signature,
				true,
				new CryptoPP::SignatureVerificationFilter(
					verifier,
					NULL,
					CryptoPP::SignatureVerificationFilter::THROW_EXCEPTION
				)
			);
			return true;

		} catch (CryptoPP::Exception& e) {
			//print::error(e.what());
			return false;
		}
	}

	bool VerifyKey(
		const std::string& _signature,
		const AES::Key& _plainKey,
		const RSA::PublicKey& _publicKey
	) {
		try {
			CryptoPP::RSASS<
				CryptoPP::PSS,
				CryptoPP::SHA256
			>::Verifier verifier(_publicKey);

			CryptoPP::SecByteBlock buffer(_plainKey.size() + _signature.size());
			std::memcpy(buffer.data(), _plainKey.data(), _plainKey.size());
			std::memcpy(buffer.data() + _plainKey.size(), _signature.data(), _signature.size());

			CryptoPP::StringSource(
				buffer.data(),
				buffer.size(),
				true,
				new CryptoPP::SignatureVerificationFilter(
					verifier,
					NULL,
					CryptoPP::SignatureVerificationFilter::THROW_EXCEPTION
				)
			);
			return true;

		} catch (CryptoPP::Exception& e) {
			//print::error(e.what());
			return false;
		}
	}
};


