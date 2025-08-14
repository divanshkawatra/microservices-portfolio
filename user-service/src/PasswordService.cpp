#include <string>
#include <vector>
#include <stdexcept>
#include <fstream>
#include <random>
#include "argon2.h" // Note double quotes, not angle braces
#include "PasswordService.h"

using namespace std;


// Takes a plaintext password and produce a secure, encoded hash string to store in the DB
// The encoded hash conveniently contains the salt, the parameters, and the final hash all in one string.
string PasswordService::hashPassword(string& pPassword){
    // 1. Define your parameters
    // t_cost: Time cost, or number of iterations.
    const uint32_t t_cost = 2;
    // m_cost: Memory cost in KiB. (1 << 16) is 65536 KiB, or 64 MiB.
    const uint32_t m_cost = (1 << 16);
    // parallelism: Number of parallel threads to use.
    const uint32_t parallelism = 1;

    // 2. Create a salt (a random value)
    // Define the salt container.
    vector<uint8_t> salt(16); // 16 bytes is a good length for a salt.

    // Create an instance of std::random_device.
    // This is our source of true, non-deterministic randomness from the OS.
    random_device rd;

    // Seed a high-quality pseudo-random number engine (Mersenne Twister)
    // with the value from the random_device.
    mt19937 engine(rd());

    // Create a uniform distribution to produce numbers in the range of a byte [0, 255].
    // We use unsigned int because some standard library implementations are fussy about char types.
    uniform_int_distribution<unsigned int> dist(0, 255);

    // Fill the salt vector with random bytes from our generator.
    for (auto& byte : salt) {
        byte = static_cast<uint8_t>(dist(engine));
    }


    const uint32_t hash_len = 32;      // Desired length of the raw hash in bytes
    // 3. Define the output buffer for the encoded hash
    // The library recommends a specific size for this.
    size_t encoded_len = argon2_encodedlen(t_cost, m_cost, parallelism, salt.size(), hash_len, Argon2_id);
    vector<char> encoded(encoded_len);

    // 4. Call the hashing function
    int result = argon2id_hash_encoded(
        t_cost, m_cost, parallelism,
        pPassword.c_str(), pPassword.length(),
        salt.data(), salt.size(),
        hash_len,
        encoded.data(), encoded.size()
    );

    // 5. Check for errors and return the encoded string
    if (result != ARGON2_OK) {
        throw std::runtime_error("Failed to hash password: " + std::string(argon2_error_message(result)));
    }

    return std::string(encoded.data());
}

// This function takes a plaintext password and an encoded hash from the database
// and tells if they match.
// The argon2id_verify function does all the hard work of extracting the salt and parameters from the hash string for you.
bool PasswordService::verifyPassword(const string& pPassword, const string& pHashedPassword) {
    // Call the verify function. It returns ARGON2_OK on success.
    int lRes = argon2id_verify(
        pHashedPassword.c_str(),  // The encoded hash from the database
        pPassword.c_str(),        // The plaintext password to check
        pPassword.size()
    );

    return (lRes == ARGON2_OK);
}