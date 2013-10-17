/*
 *  Copyright 2013 David Farrell
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "stdint.h"

struct vm_state_s
{
    uint16_t A;
    uint16_t B;
    uint16_t C;
    uint16_t X;
    uint16_t Y;
    uint16_t Z;
    uint16_t I;
    uint16_t J;

    uint16_t PC;
    uint16_t SP;
    uint16_t EX;
    uint16_t IA;

    uint16_t mem[0x10000];
};

uint16_t literals[] =
{
  0xFFFF, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
    0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E,
    0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
    0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E
};

uint16_t * get(struct vm_state_s *state, uint16_t A, unsigned char is_B)
{
    switch(A)
    {
        case 0x00:
            return &state->A;
        case 0x01:
            return &state->B;
        case 0x02:
            return &state->C;
        case 0x03:
            return &state->X;
        case 0x04:
            return &state->Y;
        case 0x05:
            return &state->Z;
        case 0x06:
            return &state->I;
        case 0x07:
            return &state->J;
        case 0x08:
            return &state->mem[state->A];
        case 0x09:
            return &state->mem[state->B];
        case 0x0A:
            return &state->mem[state->C];
        case 0x0B:
            return &state->mem[state->X];
        case 0x0C:
            return &state->mem[state->Y];
        case 0x0D:
            return &state->mem[state->Z];
        case 0x0E:
            return &state->mem[state->I];
        case 0x0F:
            return &state->mem[state->J];
        case 0x10:
            return &state->mem[state->A + *get(state, 0x1F, 0)];
        case 0x11:
            return &state->mem[state->B + *get(state, 0x1F, 0)];
        case 0x12:
            return &state->mem[state->C + *get(state, 0x1F, 0)];
        case 0x13:
            return &state->mem[state->X + *get(state, 0x1F, 0)];
        case 0x14:
            return &state->mem[state->Y + *get(state, 0x1F, 0)];
        case 0x15:
            return &state->mem[state->Z + *get(state, 0x1F, 0)];
        case 0x16:
            return &state->mem[state->I + *get(state, 0x1F, 0)];
        case 0x17:
            return &state->mem[state->J + *get(state, 0x1F, 0)];
        case 0x18:
            return is_B ? &state->mem[--state->SP]
                        : &state->mem[state->SP++];
        case 0x19:
            return &state->mem[state->SP];
        case 0x1A:
            return &state->mem[state->SP + *get(state, 0x1F, 0)];
        case 0x1B:
            return &state->SP;
        case 0x1C:
            return &state->PC;
        case 0x1D:
            return &state->EX;
        case 0x1E:
            return &state->mem[*get(state, 0x1F, 0)];
        case 0x1F:
            return &state->mem[state->PC++];
        default:
            return &literals[A - 0x20];
    }
}

void process(struct vm_state_s *state, uint16_t OP, uint16_t *VALB, uint16_t *VALA)
{
    
}

void process_special(struct vm_state_s *state, uint16_t B, uint16_t *VALA)
{
    
}

int main(int argc, char **argv)
{
    struct vm_state_s state;
    state.PC = 0;
    state.SP = 0;
    state.IA = 0;

    for(;;)
    {
        uint16_t word = *get(&state, 0x1F, 0);
        uint16_t OP = word & 0x1F;
        uint16_t B = (word >> 5) & 0x1F;
        uint16_t A = (word >> 10);

        uint16_t *VALA = get(&state, A, 0);

        if(OP == 0x0)
        {
            process_special(&state, B, VALA);
            continue;
        }

        uint16_t *VALB = get(&state, B, 1);
        process(&state, OP, VALB, VALA);
    }

    return 0;
}
