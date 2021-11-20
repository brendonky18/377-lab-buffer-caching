#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

#define WEIGHT 2
#define RAND_MAX 2147483647

/**
 * @brief pseudo-random number generator
 * @link http://www.iro.umontreal.ca/~simardr/rng/lfsr113.c
 * @return a pseudo-random 32-bit integer
 */
uint rand(void) {
  unsigned int z1 = uptime(); 
  unsigned int z2 = uptime(); 
  unsigned int z3 = uptime(); 
  unsigned int z4 = uptime();

  unsigned int b;
  b  = ((z1 << 6) ^ z1) >> 13;
  z1 = ((z1 & 4294967294U) << 18) ^ b;
  b  = ((z2 << 2) ^ z2) >> 27; 
  z2 = ((z2 & 4294967288U) << 2) ^ b;
  b  = ((z3 << 13) ^ z3) >> 21;
  z3 = ((z3 & 4294967280U) << 7) ^ b;
  b  = ((z4 << 3) ^ z4) >> 12;
  z4 = ((z4 & 4294967168U) << 13) ^ b;

  return (z1 ^ z2 ^ z3 ^ z4) / 2;
}

/**
 * @brief performs exponentiation
 * 
 * @param x the base
 * @param n the exponent
 * @return x raised to the power of n, as a long 
 */
long pow(long x, unsigned n) {
    long p = x;
    long r = 1;

    while (n > 0) {
        if (n % 2 == 1) {
            r *= p;
        }
        p *= p;
        n /= 2;
    }
    return(r);
}

int main(int argc, char const *argv[]) {
    long file_len;
    int in_fd;

    // open the file with the file path specified from the command line
    if(argc > 1){
        in_fd = open(argv[1], O_RDONLY); 
    } else {
    // open read_file if no file is specified
        in_fd = open("read_file", O_RDONLY); 
    }
    
    
    // check if file was created
    if(in_fd == -1) {
        printf(1, "No file specified, or file was not created. Usage: ./benchmark <in file path>");
        exit();
    }
    
    // get the size of the file
    struct stat buf;
    if(fstat(in_fd, &buf) < 0) {
        printf(1, "could not stat file");
        exit();
    }

    // the file being read must be less than RAND_MAX
    if(file_len > RAND_MAX) {
        printf(1, "file is too large, must me smaller than RAND_MAX");
        exit();
    }

    printf(1, "The hitrate so far is %d\n", hitrate());
    // TODO:
    // Implement the benchmark functions here. 
    // You should not have to modify any other code outside of this file

    close(in_fd);
    exit();
}

/**
 * @brief reads the file in sequential order
 * 
 * @param fd file descriptor of the file being read
 * @param read_buf buffer to store the read values
 * @param file_len the length of the file in bytes
 * @return the hit-rate while reading the file as a float
 */
float bench_sequential(int fd, char *read_buf, long file_len) {    
    resethitrate();
    // TODO:
    // Iterate over the entire twice, reading each byte in sequential order
    return hitrate();
}

/**
 * @brief reads the file randomly @see get_rand_index()
 * 
 * @param fd file descriptor of the file being read
 * @param read_buf buffer to store the read values
 * @param file_len the length of the file in bytes
 * @return the hit-rate while reading the file as a float
 */
float bench_random(int fd, char *read_buf, long file_len) {
    resethitrate();
    // TODO:
    // perform file_len reads, at random points in the file
    return hitrate();
}

/**
 * @brief reads the file randomly, weighted in favor of lower indicies @see get_wrand_index
 * 
 * @param fd file descriptor of the file being read
 * @param read_buf buffer to store the read values
 * @param file_len the length of the file in bytes
 * @return the hit-rate while reading the file as a float
 */
float bench_weighted_random(int fd, char *read_buf, long file_len) {
    resethitrate();
    // TODO:
    // perform file_len reads, at weighted random points in the file
    return hitrate();
}

/**
 * @brief Get a rand index in the range [0, len)
 * 
 * @param max the max (exclusive) random number to return
 * @return a random int in the specified range
 */
int get_rand_index(int max) {
    return rand() % (max - 1);
}

/**
 * @brief Get a weighted random index in the range [0, len).
 *        The random number is weighted by factor {@a WEIGHT}.
 *        The return value is calculated by mapping a uniformly distributed random value x, 
 *        on the curve x^WEIGHT.
 * @param max the max (exclusive) random number to return 
 * @return a weighted random int in the specified range 
 */
int get_wrand_index(int max) {
    float x = rand()/RAND_MAX;
    return (max - 1) * pow(x, WEIGHT);
}