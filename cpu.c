/*
 *  cpu.c
 *  Contains APEX cpu pipeline implementation
 *
 *  Author :
 *  Gaurav Kothari (gkothar1@binghamton.edu)
 *  State University of New York, Binghamton
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"

/* Set this flag to 1 to enable debug messages */
#define ENABLE_DEBUG_MESSAGES 1

/*
 * This function creates and initializes APEX cpu.
 *
 * Note : You are free to edit this function according to your
 * 				implementation
 */
APEX_CPU*
APEX_cpu_init(const char* filename)
{
  if (!filename) {
    return NULL;
  }

  APEX_CPU* cpu = malloc(sizeof(*cpu));
  if (!cpu) {
    return NULL;
  }

  /* Initialize PC, Registers and all pipeline stages */
  cpu->pc = 4000;
  memset(cpu->regs, 0, sizeof(int) * 32);
  memset(cpu->regs_valid, 1, sizeof(int) * 32);
  memset(cpu->stage, 0, sizeof(CPU_Stage) * NUM_STAGES);
  memset(cpu->data_memory, 0, sizeof(int) * 4000);

  /* Parse input file and create code memory */
  cpu->code_memory = create_code_memory(filename, &cpu->code_memory_size);

  if (!cpu->code_memory) {
    free(cpu);
    return NULL;
  }

  if (ENABLE_DEBUG_MESSAGES) {
    fprintf(stderr,
            "APEX_CPU : Initialized APEX CPU, loaded %d instructions\n",
            cpu->code_memory_size);
    fprintf(stderr, "APEX_CPU : Printing Code Memory\n");
    printf("%-9s %-9s %-9s %-9s %-9s\n", "opcode", "rd", "rs1", "rs2", "imm");

    for (int i = 0; i < cpu->code_memory_size; ++i) {
      printf("%-9s %-9d %-9d %-9d %-9d\n",
             cpu->code_memory[i].opcode,
             cpu->code_memory[i].rd,
             cpu->code_memory[i].rs1,
             cpu->code_memory[i].rs2,
             cpu->code_memory[i].imm);
    }
  }

  /* Make all stages busy except Fetch stage, initally to start the pipeline */
  for (int i = 1; i < NUM_STAGES; ++i) {
    cpu->stage[i].busy = 1;
  }

  return cpu;
}

/*
 * This function de-allocates APEX cpu.
 *
 * Note : You are free to edit this function according to your
 * 				implementation
 */
void
APEX_cpu_stop(APEX_CPU* cpu)
{
  free(cpu->code_memory);
  free(cpu);
}

/* Converts the PC(4000 series) into
 * array index for code memory
 *
 * Note : You are not supposed to edit this function
 *
 */
int
get_code_index(int pc)
{
  return (pc - 4000) / 4;
}

static void
print_instruction(CPU_Stage* stage)
{
  if (strcmp(stage->opcode, "STORE") == 0) {
    printf(
      "%s,R%d,R%d,#%d ", stage->opcode, stage->rs1, stage->rs2, stage->imm);
  }

  if (strcmp(stage->opcode, "MOVC") == 0) {
    printf("%s,R%d,#%d ", stage->opcode, stage->rd, stage->imm);
  }

  if (strcmp(stage->opcode, "ADD") == 0) {
    printf(
      "%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }

  if (strcmp(stage->opcode, "SUB") == 0) {
    printf(
      "%s,R%d,R%d,R%d", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }

  if (strcmp(stage->opcode, "AND") == 0) {
    printf(
      "%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }

  if (strcmp(stage->opcode, "OR") == 0) {
    printf(
      "%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }

  if (strcmp(stage->opcode, "EX-OR") == 0) {
    printf(
      "%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }

  if (strcmp(stage->opcode, "LOAD") == 0) {
    printf(
      "%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->imm);
  }

  if (strcmp(stage->opcode, "MUL") == 0) {
    printf(
      "%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }

  if (strcmp(stage->opcode, "BZ") == 0) {
    printf(
      "%s,%d ", stage->opcode, stage->imm);
  }

  if (strcmp(stage->opcode, "BNZ") == 0) {
    printf(
      "%s,%d ", stage->opcode, stage->imm);
  }

  if (strcmp(stage->opcode, "HALT") == 0) {
    printf(
    "%s ", stage->opcode);
  }

  if(strcmp(stage->opcode, "JUMP") == 0){
    printf(
      "%s,R%d,#%d ", stage->opcode, stage->rs1, stage->imm);
  }

}

/* Debug function which dumps the cpu stage
 * content
 *
 * Note : You are not supposed to edit this function
 *
 */

 static void NO_OP(char* name ) {
   printf("%-15s:  EMPTY ",name);
   printf("\n");
 }

static void
print_stage_content(char* name, CPU_Stage* stage)
{
  printf("%-15s: pc(%d) ", name, stage->pc);
  print_instruction(stage);
  printf("\n");
}

/*
 *  Fetch Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
fetch(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[F];
  if (!stage->busy && !stage->stalled) {
    /* Store current PC in fetch latch */
    stage->pc = cpu->pc;

    /* Index into code memory using this pc and copy all instruction fields into
     * fetch latch
     */
    APEX_Instruction* current_ins = &cpu->code_memory[get_code_index(cpu->pc)];
    strcpy(stage->opcode, current_ins->opcode);
    stage->rd = current_ins->rd;
    stage->rs1 = current_ins->rs1;
    stage->rs2 = current_ins->rs2;
    stage->imm = current_ins->imm;
    stage->rd = current_ins->rd;

    /* Update PC for next instruction */
    cpu->pc += 4;

    /* Copy data from fetch latch to decode latch*/
    cpu->stage[DRF] = cpu->stage[F];

    if(strcmp((cpu->simulate),"display")==0)
    {
      print_stage_content("Instruction at Fetch Stage---> :\t", stage);

    }


  }

   else
  {

    cpu->stage[DRF]=cpu->stage[F];
    NO_OP("Instruction at Fetch_Stage---> :\t");

  }

  return 0;
}

/*
 *  Decode Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
decode(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[DRF];
  if (!stage->busy && !stage->stalled) {

    /* Read data from register file for store */
    if (strcmp(stage->opcode, "STORE") == 0) {

      stage->rs1_value=cpu->regs[stage->rs1];
      //printf("\nRS1 in decode stage store=%d",stage->rs1_value);
      stage->rs2_value=cpu->regs[stage->rs2];
    }

    /* No Register file read needed for MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0) {
      //cpu->regs_valid[stage->rd]=0;
    }

   if (strcmp(stage->opcode, "ADD") == 0) {

     if(stage->rd!=stage->rs1 && stage->rd!=stage->rs2)
     {
        cpu->regs_valid[stage->rd]=1;
     }

     if(cpu->regs_valid[stage->rs1]==0 && cpu->regs_valid[stage->rs2]==0)
     {
       cpu->stage[F].stalled=0;
       cpu->stage[DRF].stalled=0;
       stage->rs1_value=cpu->regs[stage->rs1];
       //printf("\nRS1 in decode stage=%d",stage->rs1_value);
       stage->rs2_value=cpu->regs[stage->rs2];
      // printf("\nRS2 in decode stage=%d\n",stage->rs1_value);
     }
     else
     {
       cpu->stage[F].stalled=1;
       cpu->stage[DRF].stalled=1;

     }
    }

    if (strcmp(stage->opcode, "SUB") == 0) {

      if(stage->rd!=stage->rs1 && stage->rd!=stage->rs2)
      {
         cpu->regs_valid[stage->rd]=1;
      }

      if(cpu->regs_valid[stage->rs1]==0 && cpu->regs_valid[stage->rs2]==0)
      {
        cpu->stage[F].stalled=0;
        cpu->stage[DRF].stalled=0;
        stage->rs1_value=cpu->regs[stage->rs1];
        stage->rs2_value=cpu->regs[stage->rs2];

      }
      else
      {
        cpu->stage[F].stalled=1;
        cpu->stage[DRF].stalled=1;
      }

    }

    if (strcmp(stage->opcode, "AND") == 0) {

      if(stage->rd!=stage->rs1 && stage->rd!=stage->rs2)
      {
         cpu->regs_valid[stage->rd]=1;
      }

      if(cpu->regs_valid[stage->rs1]==0 && cpu->regs_valid[stage->rs2]==0)
      {
        cpu->stage[F].stalled=0;
        cpu->stage[DRF].stalled=0;
        stage->rs1_value=cpu->regs[stage->rs1];
        stage->rs2_value=cpu->regs[stage->rs2];

      }
      else
      {
        cpu->stage[F].stalled=1;
        cpu->stage[DRF].stalled=1;
      }
    }

    if (strcmp(stage->opcode, "OR") == 0) {


      if(stage->rd!=stage->rs1 && stage->rd!=stage->rs2)
      {
         cpu->regs_valid[stage->rd]=1;
      }

      if(cpu->regs_valid[stage->rs1]==0 && cpu->regs_valid[stage->rs2]==0)
      {
        cpu->stage[F].stalled=0;
        cpu->stage[DRF].stalled=0;
        stage->rs1_value=cpu->regs[stage->rs1];
        stage->rs2_value=cpu->regs[stage->rs2];
      }
      else
      {
        cpu->stage[F].stalled=1;
        cpu->stage[DRF].stalled=1;
      }
    }

    if (strcmp(stage->opcode, "EX-OR") == 0) {

      if(stage->rd!=stage->rs1 && stage->rd!=stage->rs2)
      {
         cpu->regs_valid[stage->rd]=1;
      }

      if(cpu->regs_valid[stage->rs1]==0 && cpu->regs_valid[stage->rs2]==0)
      {
        cpu->stage[F].stalled=0;
        cpu->stage[DRF].stalled=0;
        stage->rs1_value=cpu->regs[stage->rs1];
        stage->rs2_value=cpu->regs[stage->rs2];

      }
      else
      {
        cpu->stage[F].stalled=1;
        cpu->stage[DRF].stalled=1;
      }
    }

    if (strcmp(stage->opcode, "LOAD") == 0) {

      if(stage->rd!=stage->rs1 && stage->rd!=stage->rs2)
      {
         cpu->regs_valid[stage->rd]=1;
      }

      stage->rs1_value=cpu->regs[stage->rs1];
    //  stage->rs2_value=cpu->regs[stage->rs2];
      stage->rs2_value=cpu->regs[stage->imm];
    }

    if (strcmp(stage->opcode, "MUL") == 0) {

      if(stage->rd!=stage->rs1 && stage->rd!=stage->rs2)
      {
         cpu->regs_valid[stage->rd]=1;
      }

      if(cpu->regs_valid[stage->rs1]==0 && cpu->regs_valid[stage->rs2]==0)
      {
        cpu->stage[F].stalled=0;
        cpu->stage[DRF].stalled=0;
        stage->rs1_value=cpu->regs[stage->rs1];
        stage->rs2_value=cpu->regs[stage->rs2];

      }
      else
      {
        cpu->stage[F].stalled=1;
        cpu->stage[DRF].stalled=1;
      }



    }

    if (strcmp(stage->opcode, "JUMP") == 0) {

      stage->rs1_value=cpu->regs[stage->rs1];

    }

    if (strcmp(stage->opcode, "HALT") == 0) {

      cpu->stage[F].stalled=1;

    }


    /* Copy data from decode latch to execute latch*/
    cpu->stage[EX] = cpu->stage[DRF];

    if(strcmp((cpu->simulate),"display")==0)
    {
      print_stage_content("Instruction at Decode/RF Stage---> :\t", stage);

    }


  }

  else
  {

    cpu->stage[EX]=cpu->stage[DRF];
    NO_OP("Instruction at Decode/RF Stage---> :\t");

  }
  return 0;
}

/*
 *  Execute Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
execute(APEX_CPU* cpu)
{
  int stallFlag=1;  // for mul
  CPU_Stage* stage = &cpu->stage[EX];
  if (!stage->busy && !stage->stalled) {


    /* MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0) {
      stage->buffer=(stage->imm);

    }
    if (strcmp(stage->opcode, "STORE") == 0) {
      stage->mem_address=(stage->rs2_value)+(stage->imm);

    }

    if (strcmp(stage->opcode, "LOAD") == 0)
    {

        stage->mem_address=(stage->rs1_value)+(stage->imm);

    }

     /* ADD */
     if (strcmp(stage->opcode, "ADD") == 0) {

        if(cpu->regs[stage->rs1]==0)
       {
         stage->buffer=(stage->rs2_value)+(stage->imm);
       }
       else if(cpu->regs[stage->rs2]==0)
       {
         stage->buffer=(stage->rs1_value)+(stage->imm);
       }
       else
       {
         stage->buffer=(stage->rs1_value)+(stage->rs2_value);
       }



       if(stage->buffer==0)    // for handling BZ and BNZ
       {
         cpu->zeroflag=1;
       }
       else
       {
         cpu->zeroflag=0;
       }

     }


     /* SUB */
    if (strcmp(stage->opcode, "SUB") == 0) {

     if(stage->rs1==0)
      {
        stage->buffer=(stage->rs2_value)-(stage->imm);
      }

      else if(stage->rs2==0)
      {
        stage->buffer=(stage->rs1_value)-(stage->imm);
      }

      else
      {
        stage->buffer=(stage->rs1_value)-(stage->rs2_value);
      }

        if(stage->buffer==0)    // for handling BZ and BNZ
        {
          cpu->zeroflag=1;
        }
        else
        {
        cpu->zeroflag=0;
        }

    }


    if (strcmp(stage->opcode, "AND") == 0) {
      stage->buffer=(stage->rs1_value)&(stage->rs2_value);
    }

    if (strcmp(stage->opcode, "OR") == 0) {
      stage->buffer=(stage->rs1_value)|(stage->rs2_value);
    }

    if (strcmp(stage->opcode, "EX-OR") == 0) {
      stage->buffer=(stage->rs1_value)^(stage->rs2_value);
    }


    if (strcmp(stage->opcode, "MUL") == 0) {

        if(stallFlag==1)
        {
          stallFlag=stallFlag+1;
          cpu->clock++;
        }

        if(stallFlag==2)
        {
            stage->buffer=(stage->rs1_value)*(stage->rs2_value);
            stage->stalled=0;
                                        //printf("Multiplication executed %d",stage->rs2_value);
            stallFlag=0;
        }

        if(stage->buffer==0)    // for handling BZ and BNZ
        {
          cpu->zeroflag=1;
        }
        else
        {
        cpu->zeroflag=0;
        }

    }



if (strcmp(stage->opcode, "BZ") == 0) {

    if(cpu->zeroflag==1)
    {
      cpu->pc= stage->pc + stage->imm;
      cpu->zeroflag=0;
    }
}

if (strcmp(stage->opcode, "BNZ") == 0) {

    if(cpu->zeroflag==0)
    {
      cpu->pc= stage->pc + stage->imm;
    }

}

if(strcmp(stage->opcode, "JUMP") == 0){

  cpu->pc=stage->rs1_value + stage->imm;
}

if(strcmp(stage->opcode, "HALT") == 0){

}

    /* Copy data from Execute latch to Memory latch*/
    cpu->stage[MEM] = cpu->stage[EX];

    if(strcmp((cpu->simulate),"display")==0)
    {
      print_stage_content("Instruction at Execute Stage---> :\t", stage);

    }


  }

 else
  {

    cpu->stage[MEM]=cpu->stage[EX];
    NO_OP("Instruction at Execute Stage---> :\t");

  }

  return 0;
}


/*
 *  Memory Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
memory(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[MEM];
  if (!stage->busy && !stage->stalled) {

    /* Store */
    if (strcmp(stage->opcode, "STORE") == 0)
    {
      cpu->data_memory[stage->mem_address]=cpu->regs[stage->rs1];
    }

    if (strcmp(stage->opcode, "LOAD") == 0)
    {
      stage->buffer=cpu->data_memory[stage->mem_address];
    }

    if (strcmp(stage->opcode, "ADD") == 0)
    {

    }
    if (strcmp(stage->opcode, "SUB") == 0)
    {

    }
    if (strcmp(stage->opcode, "MUL") == 0)
    {

    }
    if (strcmp(stage->opcode, "OR") == 0)
    {

    }
    if (strcmp(stage->opcode, "EX_OR") == 0)
    {

    }
    if (strcmp(stage->opcode, "AND") == 0)
    {

    }

    /* Copy data from decode latch to execute latch*/
    cpu->stage[WB] = cpu->stage[MEM];

    if(strcmp((cpu->simulate),"display")==0)
    {
      print_stage_content("Instruction at Memory Stage---> :\t", stage);

    }


  }

   else
  {

    cpu->stage[WB]=cpu->stage[MEM];
    NO_OP("Instruction at Memory Stage---> :\t");

  }

  return 0;
}

/*
 *  Writeback Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
writeback(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[WB];
  if (!stage->busy && !stage->stalled) {

    /* Update register file */
    if (strcmp(stage->opcode, "MOVC") == 0) {
      cpu->regs[stage->rd] = stage->buffer;
      cpu->regs_valid[stage->rd]=0;
      cpu->stage[DRF].stalled=0;
      cpu->stage[F].stalled=0;
    }

    if (strcmp(stage->opcode, "ADD") == 0) {

      cpu->regs[stage->rd] = stage->buffer;
      cpu->regs_valid[stage->rd]=0;
      cpu->stage[DRF].stalled=0;
      cpu->stage[F].stalled=0;
    }

    if (strcmp(stage->opcode, "SUB") == 0) {

      cpu->regs[stage->rd] = stage->buffer;
      cpu->regs_valid[stage->rd]=0;
      cpu->stage[DRF].stalled=0;
      cpu->stage[F].stalled=0;
    }

    if (strcmp(stage->opcode, "AND") == 0) {

      cpu->regs[stage->rd] = stage->buffer;
      cpu->regs_valid[stage->rd]=0;
      cpu->stage[DRF].stalled=0;
      cpu->stage[F].stalled=0;
    }

    if (strcmp(stage->opcode, "EX-OR") == 0) {

      cpu->regs[stage->rd] = stage->buffer;
      cpu->regs_valid[stage->rd]=0;
      cpu->stage[DRF].stalled=0;
      cpu->stage[F].stalled=0;
    }

    if (strcmp(stage->opcode, "OR") == 0) {

      cpu->regs[stage->rd] = stage->buffer;
      cpu->regs_valid[stage->rd]=0;
      cpu->stage[DRF].stalled=0;
      cpu->stage[F].stalled=0;
    }

    if (strcmp(stage->opcode, "LOAD") == 0) {

      cpu->regs[stage->rd] = stage->buffer;
      cpu->regs_valid[stage->rd]=0;
      cpu->stage[DRF].stalled=0;
      cpu->stage[F].stalled=0;
    }


    if (strcmp(stage->opcode, "MUL") == 0) {

      cpu->regs[stage->rd] = stage->buffer;
      cpu->regs_valid[stage->rd]=0;
      cpu->stage[DRF].stalled=0;
      cpu->stage[F].stalled=0;
    }


    cpu->ins_completed++;

    if(strcmp((cpu->simulate),"display")==0)
    {
      print_stage_content("Instruction at Writeback Stage---> :\t", stage);

    }


  }

  else
  {

    //cpu->stage[DRF]=cpu->stage[F];
    NO_OP("Instruction at Writeback Stage---> :\t");

  }

  return 0;
}

void Memorydisplay(APEX_CPU* cpu)
{
    for(int i=0; i<100; i++)
    {
      printf("\n |     MEM[%d]     |    Data Value == %d \n", i , cpu->data_memory[i]);
    }
}

void display(APEX_CPU* cpu)
{
    for(int i=0;i<16;i++)
    {
          printf("\n R%d=%d",i,cpu->regs[i]);
          printf("\n |      REG[%d]      |       Value = %d          |\n",i,cpu->regs[i]);
    }
}

/*
 *  APEX CPU simulation loop
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
 int
 APEX_cpu_run(APEX_CPU* cpu)
 {
   while (1) {

     /* All the instructions committed, so exit */
     if (cpu->ins_completed == cpu->code_memory_size /* || cpu->clock==cpu->clock_cycle  */) {
       printf("(apex) >> Simulation Complete");
       break;
     }

     if (ENABLE_DEBUG_MESSAGES) {
     //  printf("\n--------------------------------\n");
       printf("\n--------------------------------Clock Cycle #: %d--------------------------------\n\n", cpu->clock);
       //printf("--------------------------------\n");
     }

     writeback(cpu);
     memory(cpu);
     execute(cpu);
     decode(cpu);
     fetch(cpu);
     cpu->clock++;
   }


   printf("\n\n\n=============== STATE OF ARCHITECTURAL REGISTER FILE ==========");
   display(cpu);


   printf("\n\n\n============== STATE OF DATA MEMORY =============\n\n");
   Memorydisplay(cpu);

   return 0;
 }
