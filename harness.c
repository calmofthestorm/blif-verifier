#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>

const extern int numInputs;
const extern int numOutputs;

const extern char* INPUT_NAMES[];
const extern char* OUTPUT_NAMES[];

void circuit0(size_t inputs[numInputs], size_t outputs[numOutputs]);
void circuit1(size_t inputs[numInputs], size_t outputs[numOutputs]);

unsigned int min(unsigned int a, unsigned int b) {
  return (a < b) ? a : b;
}

int main() {
  const size_t word_length = sizeof(size_t) * 8;

  // Bit-parallel selection structures. Each input has two states it toggles
  // between; to simulate all vectors we simulate all combinations of states
  // (essentially the states are generalizations of 0 and 1).
  size_t inputs[numInputs];
  size_t outputs[2][numOutputs];
  int static_inputs = min((unsigned int)(log(word_length) / log(2)), numInputs);

  // Populate static outputs. This code should be flexible across
  // word size, but does assume integer length in bits is a power
  // of two.
  size_t period = 2;
  size_t seed = 2;

  int i = 0;
  // These inputs never change value
  for (; i < static_inputs; i++) {
    inputs[i] = 0;
    int j;
    for (j = 0; j < word_length / period; j++) {
      inputs[i] = (inputs[i] << period) + (seed - 1);
    }
    period *= 2;
    seed *= seed;
  }

  // These inputs will "count" from all 0's to all 1's.
  for (; i < numInputs; i++) {
    inputs[i] = 0;
  }
  
  /*
  for (i = 0; i < numInputs; i++) {
    printf("%.8x %.8x\n", (inputs[i] >> 32) & 0x00000000FFFFFFFF, inputs[i] & 0x00000000FFFFFFFF);
  }
  */

  unsigned long long sims = 0;
  while (1) {
    /*j
    printf("FUN: ");
    {
      int i;
      for (i = 0; i < numInputs; i++) {
        printf("%.16llx ", inputs[i]);
      }
    }
    printf("\n");
    */

    // Verify that the circuits are equivalent
    circuit0(inputs, outputs[0]);
    circuit1(inputs, outputs[1]);
    int violated = 0;
    int i;
    for (i = 0; !violated && i < numOutputs; i++) {
      if (outputs[0][i] != outputs[1][i]) {
        printf("Circuits are NOT equivalent. Contradiction on output %s\n", OUTPUT_NAMES[i]);
        size_t j;
        for (j = 1; j != 0; j <<= 1) {
          if ((outputs[0][i] & j) != (outputs[1][i] & j)) {
            int k;
            for (k = 0; k < numInputs; k++) {
              printf("%s: %i\n", INPUT_NAMES[k], 0 != (inputs[k] & j));
            }
            printf("Inputs: ");
            for (k = numInputs - 1; k >= 0; k--) {
              printf("%i", (inputs[k] & j) != 0);
            }
            printf("\n\nCircuit 0 output: ");
            for (k = numOutputs - 1; k >= 0; k--) {
              printf("%i", (outputs[0][i] & j) != 0);
            }
            printf("\nCircuit 1 output: ");
            for (k = numOutputs - 1; k >= 0; k--) {
              printf("%i", (outputs[1][i] & j) != 0);
            }
            printf("\n");
            break;
          }
        }
        return 1;
      }
    }

    // Increment the test vector (starting with the first bit that changes)
    unsigned int ind = static_inputs;
    while (ind < numInputs && inputs[ind] != 0) {
      inputs[ind] = 0;
      ind++;
    }
    sims += word_length;
    if (ind == numInputs) {
      printf("Circuits are equivalent. %lli combinations explored.\n", sims);
      return 0;
    }
    inputs[ind] = (size_t)(-1);
  }
}
