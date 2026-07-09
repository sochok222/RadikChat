#include "hash_functions.h"

#include "openssl/err.h"
#include "openssl/evp.h"
#include "openssl/types.h"

#define CLEAR_SSL_ERROR_QUEUE() { while (ERR_get_error()); }

uint8_t *sha256_hash(uint8_t *input, size_t input_length, size_t *output_length)
{
    EVP_MD_CTX *ctx = NULL;
    EVP_MD *sha256 = NULL;
    unsigned char *out_digest = NULL;
    unsigned int len = 0;

    /* Create a context for the hash operation */
    ctx = EVP_MD_CTX_new();
    if (ctx == NULL)
        goto err;

    /*
     * Fetch the SHA256 algorithm implementation for doing the hash. We're
     * using the "default" library context here (first NULL parameter), and
     * we're not supplying any particular search criteria for our SHA256
     * implementation (second NULL parameter). Any SHA256 implementation will
     * do.
     * In a larger application this fetch would just be done once, and could
     * be used for multiple calls to other operations such as EVP_DigestInit_ex().
     */
    sha256 = EVP_MD_fetch(NULL, "SHA256", NULL);
    if (sha256 == NULL)
        goto err;

    /* Initialize the hash operation */
    if (!EVP_DigestInit_ex(ctx, sha256, NULL))
        goto err;

    /*
     * Pass the data to be hashed. This can be passed in over multiple
     * EVP_DigestUpdate calls if necessary
     */
    if (!EVP_DigestUpdate(ctx, input, input_length))
        goto err;

    /* Allocate the output buffer */
    out_digest = OPENSSL_malloc(EVP_MD_get_size(sha256));
    if (out_digest == NULL)
        goto err;

    /* Now calculate the hash itself */
    if (!EVP_DigestFinal_ex(ctx, out_digest, (unsigned int*)&output_length))
        goto err;

    EVP_MD_free(sha256);
    EVP_MD_CTX_free(ctx);
    return out_digest;

    err:
    OPENSSL_free(out_digest);
    EVP_MD_free(sha256);
    EVP_MD_CTX_free(ctx);
    return NULL;
}
