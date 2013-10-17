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

#include <stdio.h>
#include <stdint.h>

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

    unsigned char skip;
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
    if(state->skip)
    {
        if(OP < 0x10 || OP > 0x17) state->skip = 0;
        return;
    }

    switch(OP)
    {
        case 0x01: // SET b,a
            *VALB = *VALA;
            break;
        case 0x02: // ADD b,a
            *VALB += *VALA;
            state->EX = *VALB < *VALA;
            break;
        case 0x03: // SUB b,a
            *VALB -= *VALA;
            state->EX = *VALB >= 0 - *VALA;
            break;
        case 0x04: // MUL b,a
            state->EX = ((*VALB * *VALA) >> 16) & 0xFFFF;
            *VALB *= *VALA;
            break;
        case 0x05: // MLI b,a
            state->EX = ((*(int16_t *)VALB * *(int16_t *)VALA) >> 16) & 0xFFFF;
            *(int16_t *)VALB *= *(int16_t *)VALA;
            break;
        case 0x06: // DIV b,a
            if(*VALA == 0)
            {
                *VALB = 0;
                break;
            }

            state->EX = ((*VALB << 16) / *VALA) & 0xFFFF;
            *VALB /= *VALA;
            break;
        case 0x07: // DVI b,a
            if(*(int16_t *)VALA == 0)
            {
                *(int16_t *)VALB = 0;
                break;
            }

            state->EX = ((*(int16_t *)VALB << 16) / *(int16_t *)VALA) & 0xFFFF;
            *(int16_t *)VALB /= *(int16_t *)VALA;
            break;
        case 0x08: // MOD b,a
            *VALB = (*VALA == 0) ? 0 : *VALB % *VALA;
            break;
        case 0x09: // MDI b,a
            *(int16_t *)VALB = (*(int16_t *)VALA == 0) ? 0
                             : *(int16_t *)VALB % *(int16_t *)VALA;
            break;
        case 0x0A: // AND b,a
            *VALB &= *VALA;
            break;
        case 0x0B: // BOR b,a
            *VALB |= *VALA;
            break;
        case 0x0C: // XOR b,a
            *VALB ^= *VALA;
            break;
        case 0x0D: // SHR b,a
            state->EX = ((*VALB << 16) >> *VALA) & 0xFFFF;
            *VALB >>= *VALA;
            break;
        case 0x0E: // ASR b,a
            state->EX = ((*(int16_t *)VALB << 16) >> *VALA) & 0xFFFF;
            *(int16_t *)VALB >>= *VALA;
            break;
        case 0x0F: // SHL b,a
            state->EX = ((*VALB << *VALA) >> 16) & 0xFFFF;
            *VALB <<= *VALA;
            break;
        // TODO
        case 0x10: // IFB b,a
            state->skip = !((*VALB & *VALA) != 0);
            break;
        case 0x11: // IFC b,a
            state->skip = !((*VALB & *VALA) == 0);
            break;
        case 0x12: // IFE b,a
            state->skip = !(*VALB == *VALA);
            break;
        case 0x13: // IFN b,a
            state->skip = !(*VALB != *VALA);
            break;
        case 0x14: // IFG b,a
            state->skip = !(*VALB > *VALA);
            break;
        case 0x15: // IFA b,a
            state->skip = !(*(int16_t *)VALB > *(int16_t *)VALA);
            break;
        case 0x16: // IFL b,a
            state->skip = !(*VALB < *VALA);
            break;
        case 0x17: // IFU b,a
            state->skip = !(*(int16_t *)VALB < *(int16_t *)VALA);
            break;
        // TODO
        case 0x1A: // ADX b,a
            break;
        case 0x1B: // SBX b,a
            break;
        case 0x1E: // STI b,a
            *VALB = *VALA;
            ++state->I;
            ++state->J;
            break;
        case 0x1F: // STD b,a
            *VALB = *VALA;
            --state->I;
            --state->J;
            break;
    }
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
    state.skip = 0;

    // SET [0x2000],0x4
    state.mem[0x0000] = 0x97C1; // [100011 11][110 00001]
    state.mem[0x0001] = 0x2000;

    // ADD [0x2000],0x1
    state.mem[0x0002] = 0x7FC2; // [011111 11][110 00010]
    state.mem[0x0003] = 0x0001;
    state.mem[0x0004] = 0x2000;

    // IFE A,A
    state.mem[0x0005] = 0x0012;
    // ADD [0x2000],0x3
    state.mem[0x0006] = 0x7FC2; // [011111 11][110 00010]
    state.mem[0x0007] = 0x0003;
    state.mem[0x0008] = 0x2000;

    // IFN A,A
    state.mem[0x0009] = 0x0013;
    // ADD [0x2000],0xF
    state.mem[0x000A] = 0x7FC2; // [011111 11][110 00010]
    state.mem[0x000B] = 0x000F;
    state.mem[0x000C] = 0x2000;

    // IFN A,A
    state.mem[0x000D] = 0x0013;
    // IFE A,A
    state.mem[0x000E] = 0x0012;
    // ADD [0x2000],0xF
    state.mem[0x000F] = 0x7FC2; // [011111 11][110 00010]
    state.mem[0x0010] = 0x000F;
    state.mem[0x0011] = 0x2000;

    for(;;)
    {
        fputc('\n', stderr);
        fprintf(stderr, "PC=0x%.*x\n", 4, state.PC);
        fprintf(stderr, "[0x2000]=0x%.*x\n", 4, state.mem[0x2000]);

        uint16_t word = *get(&state, 0x1F, 0);
        uint16_t OP = word & 0x1F;
        uint16_t B = (word >> 5) & 0x1F;
        uint16_t A = (word >> 10);

        fprintf(stderr, "OP=0x%.*x\n", 2, OP);
        fprintf(stderr, "B=0x%.*x\n", 2, B);
        fprintf(stderr, "A=0x%.*x\n", 2, A);

        uint16_t *VALA = get(&state, A, 0);

        if(OP == 0)
        {
            process_special(&state, B, VALA);
            continue;
        }

        uint16_t *VALB = get(&state, B, 1);
        process(&state, OP, VALB, VALA);
    }

    return 0;
}
