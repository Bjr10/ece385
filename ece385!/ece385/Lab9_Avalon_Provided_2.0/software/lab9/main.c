/************************************************************************
 Lab 9 Nios Software
 Dong Kai Wang, Fall 2017
 Christine Chen, Fall 2013
 For use with ECE 385 Experiment 9
 University of Illinois ECE Department
 ************************************************************************/
 #include <stdlib.h>
 #include <stdio.h>
 #include <time.h>
 #include "aes.h"
 #define  FIRST   0xFF000000
 #define  BEGIN   0xF0000000
 #define THREE 0x000000FF
 #define FOUR 0x0000000F
 // Pointer to base address of AES module, make sure it matches Qsys
 volatile unsigned int * AES_PTR = (unsigned int *) 0x00000040;
 // Execution mode: 0 for testing, 1 for benchmarking
 int run_mode = 0;
 /** charToHex
  *  Convert a single character to the 4-bit value it represents.
  *
  *  Input: a character c (e.g. 'A')
  *  Output: converted 4-bit value (e.g. 0xA)
  */
 char charToHex(char c)
 {
  char hex = c;
  if (hex >= '0' && hex <= '9')
   hex -= '0';
  else if (hex >= 'A' && hex <= 'F')
  {
   hex -= 'A';
   hex += 10;
  }
  else if (hex >= 'a' && hex <= 'f')
  {
   hex -= 'a';
   hex += 10;
  }
  return hex;
 }
 /** charsToHex
  *  Convert two characters to byte value it represents.
  *  Inputs must be 0-9, A-F, or a-f.
  *
  *  Input: two characters c1 and c2 (e.g. 'A' and '7')
  *  Output: converted byte value (e.g. 0xA7)
  */
 char charsToHex(char c1, char c2)
 {
  char hex1 = charToHex(c1);
  char hex2 = charToHex(c2);
  return (hex1 << 4) + hex2;
 }
 /** encrypt
  *  Perform AES encryption in software.
  *
  *  Input: msg_ascii - Pointer to 32x 8-bit char array that contains the input message in ASCII format
  *         key_ascii - Pointer to 32x 8-bit char array that contains the input key in ASCII format
  *  Output:  msg_enc - Pointer to 4x 32-bit int array that contains the encrypted message
  *               key - Pointer to 4x 32-bit int array that contains the input key
  */
 //void encrypt(unsigned char * msg_ascii, unsigned char * key_ascii, unsigned int * msg_enc, unsigned int * key)

 void mix_col(uchar *mix_key){
  uchar t0;
  uchar t1;
  uchar t2;
  uchar t3;
  uchar matrix[16];
  int a;
  for( a= 0; a<4;a++){
   t0= mix_key[4*a];
   t1= mix_key[4*a+1];
   t2= mix_key[4*a+2];
   t3= mix_key[4*a+3];
   matrix[4*a]    =  gf_mul[t0][0]^gf_mul[t1][1]^t2^t3;
   matrix[4*a+1]  =  t0^gf_mul[t1][0]^gf_mul[t2][1]^t3;
   matrix[4*a+2]  =  t0^t1^gf_mul[t2][0]^gf_mul[t3][1];
   matrix[4*a+3] =  gf_mul[t0][1]^t1^t2^gf_mul[t3][0];

  }
  for(a= 0; a<16; a++){
   mix_key[a]= matrix[a];
  }
 }

 void Shift_row(uchar*  key){
  uchar temp[2];
  temp[0] = key[1];
  key[1]  = key[5];
  key[5]  = key[9];
  key[9]  = key[13];
  key[13] = temp[0];

  temp[0] =  key[2];
  temp[1] =  key[6];
  key[6]  =  key[14];
  key[14] =  temp[1];
  key[2]  =  key[10];
  key[10] =  temp[0];
  temp[0] =  key[15];
  key[15] =  key[11];
  key[11] =  key[7];
  key[7]  =  key[3];
  key[3]  =  temp[0];

 }
 unsigned long Rot_word( unsigned long sentence){
  unsigned long temp = (FIRST & sentence) >> 24;
 // printf("\n rot \n");
 // printf("%08x", ((sentence<< 8) | temp));
   return (sentence<< 8) | temp;

 }

 unsigned long Sub_word(unsigned long sentence){
   int i;
   unsigned long new=0;
   unsigned char a, b;
   for (i=3; i>=0; i--){
   b = (unsigned char) ((sentence >> (i * 8)) & FOUR);
    a = (unsigned char) ((sentence >> (i * 8 + 4)) & FOUR);
   new = new | (((unsigned long) aes_sbox[a*16 + b] & THREE) << (i * 8));
   }
  // printf("\n sub \n");
      //   printf("%08x\n", new);
   return new;
 }
 void KeyExpansion(unsigned long * key, unsigned long * keyschedule)
  {
  unsigned long temp;
   int i ;

   for (i = 0; i < 4; i++)
   {
    keyschedule[i] = key[i];
   // printf("%08x",key[i]);
   }
   i = 4;
   while (i < 4 * (10 + 1))
   {
    temp = keyschedule[i-1];
    if (i % 4 == 0)
    {
     temp=Sub_word(Rot_word(temp));
     temp = temp ^ Rcon[i / 4];
    }
    keyschedule[i] = keyschedule[i - 4] ^ temp;
    i++;
   }
  }
 void split(unsigned long*key_schedule, unsigned char *real_key_schedule, int a){
     int i,j;
     for( i =0;i<a;i++){
         for(j=0;j<4;j++){
             real_key_schedule[4*i+j]=key_schedule[i]>>(8*(3-j));
         }
     }
 }
 void merge(unsigned char*key_schedule, unsigned long *real_key_schedule){
     int i;
     for( i =0;i<4;i++){
             real_key_schedule[i]=(key_schedule[4*i]<<24)|(key_schedule[4*i+1]<<16)|(key_schedule[4*i+2]<<8)|(key_schedule[4*i+3]);
     }
 }
 void addroundkey(uchar * key , uchar *keyschedule, int round){
   int a;
  for(a =0; a<4;a++){
   key[4*a]   ^= keyschedule[4*a+round*16] ;
   key[4*a+1] ^= keyschedule[4*a+round*16+1] ;
   key[4*a+2] ^= keyschedule[4*a+round*16+2] ;
   key[4*a+3] ^= keyschedule[4*a+round*16+3] ;
  }
 }
 void encrypt(unsigned char * msg_ascii, unsigned char * key_ascii, unsigned int * msg_enc, unsigned int * key)
 {
  // Implement this function
  unsigned char real_state[16];
  unsigned char key_temp[16];
  unsigned long key_in [4];
  unsigned long key_schedule[44];
  unsigned long merge_state[4];
  unsigned char real_key_schedule[176];
  int a,b;
    for(b = 0;b<4; b++ ){
       for( a=0; a<4;a++ ){
        real_state[4*b+a]= charsToHex(msg_ascii[(4*b+a)*2], msg_ascii[(4*b+a)*2+1] );
        key_temp[4*b+a]= charsToHex(key_ascii[(4*b+a)*2], key_ascii[(4*b+a)*2+1] );
       }
    }
   merge(key_temp,key_in );
     for(a=0; a<4;a++){
      key[a]=key_in[a];
          }
      KeyExpansion(key_in, key_schedule);
     //printf("KEY EXPANSION: \n");
  // print_key_schedule(key_schedule);
      split(key_schedule,real_key_schedule,44);
     // print__real_key_schedule( real_key_schedule);
   //   split(state,real_state,4);
      addroundkey(real_state,real_key_schedule ,  0);
      //for(int a=0; a<16;a++){
      //          printf("%08x ",real_state[a]);
      //    }
      for(a =1;  a<10;a++){
           merge(real_state,merge_state );

           for( b=0; b<4;b++){
               merge_state[b]=Sub_word(merge_state[b]);
          }
          split(merge_state,real_state,4);
          Shift_row(real_state);
          mix_col(real_state);
          addroundkey(real_state,real_key_schedule,a);
          merge(real_state,merge_state );
      }
      for(a=0; a<4;a++){
               merge_state[a]=Sub_word(merge_state[a]);
      }
          split(merge_state,real_state,4);

       Shift_row(real_state);
      addroundkey(real_state,real_key_schedule,10);
      merge(real_state,merge_state );
      for(a=0; a<4;a++){
       msg_enc[a]=merge_state[a];
        }

 }
 /** decrypt
  *  Perform AES decryption in hardware.
  *
  *  Input:  msg_enc - Pointer to 4x 32-bit int array that contains the encrypted message
  *              key - Pointer to 4x 32-bit int array that contains the input key
  *  Output: msg_dec - Pointer to 4x 32-bit int array that contains the decrypted message
  */
 void decrypt(unsigned int * msg_enc, unsigned int * msg_dec, unsigned int * key)
 {
  // Implement this function

	 AES_PTR[14]=0;
	 AES_PTR[0]=key[3];//0xfdc2be03;//;////key[3];
	 AES_PTR[1]=key[2];//0xd613a16b;//key[2];////key[2];
	 AES_PTR[2]=key[1];//0xbeaac269;//key[1];////key[1];
     AES_PTR[3]=key[0];//0x3b280014;//key[0];///key[0];

    // printf("%08x\n",key[3]);
   //  printf("%08x",key[2]);
    // printf("%08x",key[2]);
   //  printf("%08x",key[3]);
  //      printf("%08x",msg_enc[0]);

 //        printf("%08x",msg_enc[1]);
 //       printf("%08x",msg_enc[2]);
  //      printf("%08x",msg_enc[3]);

	 AES_PTR[4]=msg_enc[3];//0xf59fcf63;//msg_enc[3];  //
	 AES_PTR[5]=msg_enc[2];//0x61019634;//msg_enc[2];////
	 AES_PTR[6]=msg_enc[1];//0x20ce4156;//msg_enc[1];//0x20ce4156;//
	 AES_PTR[7]=msg_enc[0];//0x439d6199;//msg_enc[0];//0x439d6199;//

	 AES_PTR[14]=1;

	 while(AES_PTR[15]==0){

	 }

	 msg_dec[3]= AES_PTR[8] ; //msg_dec[0]
	 msg_dec[2]= AES_PTR[9]; //msg_dec[1]
	 msg_dec[1]= AES_PTR[10];
	 msg_dec[0]= AES_PTR[11];
	 AES_PTR[14]=0;

 }
 /** main
  *  Allows the user to enter the message, key, and select execution mode
  *
  */
 int main()
 {
  // Input Message and Key as 32x 8-bit ASCII Characters ([33] is for NULL terminator)
  unsigned char msg_ascii[33];
  unsigned char key_ascii[33];
  // Key, Encrypted Message, and Decrypted Message in 4x 32-bit Format to facilitate Read/Write to Hardware
  unsigned int key[4];
  unsigned int msg_enc[4];
  unsigned int msg_dec[4];
  printf("Select execution mode: 0 for testing, 1 for benchmarking: ");
  scanf("%d", &run_mode);
  if (run_mode == 0) {
   // Continuously Perform Encryption and Decryption
   while (1) {
    int i = 0;
    printf("\nEnter Message:\n");
    scanf("%s", msg_ascii);
    printf("\n");
    printf("\nEnter Key:\n");
    scanf("%s", key_ascii);
    printf("\n");
    encrypt(msg_ascii, key_ascii, msg_enc, key);
    printf("\nEncrpted message is: \n");
    for(i = 0; i < 4; i++){
     printf("%08x", msg_enc[i]);
    }
    printf("\n");

    decrypt(msg_enc, msg_dec, key);


    printf("\nDecrypted message is: \n");
    for(i = 0; i < 4; i++){
     printf("%08x", msg_dec[i]);
    }
    printf("\n");
   }
  }
  else {
   // Run the Benchmark
   int i = 0;
   int size_KB = 2;
   // Choose a random Plaintext and Key
   for (i = 0; i < 32; i++) {
    msg_ascii[i] = 'a';
    key_ascii[i] = 'b';
   }
   // Run Encryption
   clock_t begin = clock();
   for (i = 0; i < size_KB * 64; i++)
    encrypt(msg_ascii, key_ascii, msg_enc, key);
   clock_t end = clock();
   double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
   double speed = size_KB / time_spent;
   printf("Software Encryption Speed: %f KB/s \n", speed);
   // Run Decryption
   begin = clock();
   for (i = 0; i < size_KB * 64; i++)
    decrypt(msg_enc, msg_dec, key);
   end = clock();
   time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
   speed = size_KB / time_spent;
   printf("Hardware Encryption Speed: %f KB/s \n", speed);
  }
  return 0;
 }
