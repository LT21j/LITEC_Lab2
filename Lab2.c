/* 
    Names:John Sayour, Matt Pistacchio
    Section: 03
    Date: 2/25/2014
    File name: Lab2
    Program description: Pod Racer game with two players. (For a full description read the documentation)
*/
#include <c8051_SDCC.h>// include files. This file is available online
#include <stdio.h>
#include <stdlib.h>

//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------
void Port_Init(void);      // Initialize ports for input and output
void Timer_Init(void);     // Initialize Timer 0 
void Interrupt_Init(void); //Initialize interrupts
void Timer0_ISR(void) __interrupt 1;
void Set_outputs(void); // function to set outputs
unsigned char Cases_match(unsigned char);
unsigned char Check_inputs(unsigned char); //function to check inputs
//bool Correct_answer(unsigned char);
unsigned char random(unsigned char);//get a random number that is too n-1
void Print_Score(unsigned char rounds, unsigned char pointsr, unsigned char pointsg, unsigned char  errorsr, unsigned char errorsg); 
void ADC_Init(void); //inits ADC
unsigned char read_AD_input(unsigned char n); //reads ad input

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------
__sbit __at 0xA4 BILED11; // BILED1 associated with Port 2.4
__sbit __at 0xA5 LEDA;  // Amber LED associated with Port 2.5
__sbit __at 0xA0 SS;   // Slide switch, associated with Port 2.0
__sbit __at 0xA3 SS1;   // Slide switch, associated with Port 2.3
__sbit __at 0xA2 BILED10;   // BILED1 associated with Port 2.2
__sbit __at 0xA1 LEDY;   // Yellow LED associated with Port 2.1
__sbit __at 0xB7 Buzzer;   //Buzzer associated with Port 3.7
__sbit __at 0xB6 LEDR;   //Red LED associated with Port 3.6
__sbit __at 0xB4 BILED01;   // BILED0 associated with Port 3.4
__sbit __at 0xB5 LEDG;   //Green LED associated with Port 3.5
__sbit __at 0xB2 PB2;   // Push Button 2 associated with Port 3.2
__sbit __at 0xB3 BILED00;   // BILED0 associated with Port 3.3
__sbit __at 0xB1 PB0;  // Push Button 0, associated with Port 3, Pin 1
__sbit __at 0xB0 PB1; // Push Button 1, associated with Port 3 Pin 0

unsigned int count = 0; //the main counter used to counter timer overflows
unsigned int r = 0;  //the random variable that is generated from 0-6 
void main(void) 
{
	unsigned int oldr = 0; //saves the last r variable to make sure r isn't repeated
	unsigned int result; //the result of the adc conversion is stored in this variable
	unsigned int tmax; //the max time value that is alowed
	//keeps track of the errors for the round and the players
 	unsigned char errors, errorsr, errorsg;
	//variable that keeps track of the current round
	unsigned char rounds;
	//tells whose turn its on 
	unsigned char i;
	//variable where each bit represents a push button being pressed or not. 
	unsigned char inputCase;
	//variables that keep track of the points for each player for the round and the total points 
	unsigned char pointsr,pointsrt, pointsgt, pointsg; 
	//keeps track of the last slideswitch state
	unsigned char startState;
	//the number of correct responces in a turn
	unsigned char turns;
 	Sys_Init(); // Initialize the C8051 board 
 	Port_Init(); // Configure P1.0 for analog input 
 	ADC_Init(); // Initialize A/D conversion 
	Interrupt_Init();
    Timer_Init();    // Initialize Timer 0 
	putchar(' ');    // the quote fonts may not copy correctly into SiLabs IDE

 	while (1) 
 	{ 
		LEDA = 1;  //
		startState = SS;
		while(startState == SS); //wait till slide switch is turned on
		LEDA = 0;
		result = read_AD_input(7); //read the current ad
		tmax = (result*4.90196+750); //set tmax to miliseconds
		printf("\n\rStarting Period: %ims \n\r", tmax); //print out the starting period
		TR0=1;
		rounds=0;  //set the rounds to 0 (start of a new game)
		pointsrt=0; //set both players total points to 0
 		pointsgt=0;
		r=-1;  //set r as a case that insn't a random number that can be generated
		oldr = -1; //set oldr as a case that isn't a random number that can be generated 
		while(rounds < 3)
		{
			printf("\r\n"); //add a space line
			i=0;
			//reset errors
			errorsr=errorsg=0;
			//reset points
			pointsr=0;
			pointsg=0;
			//loops through both players
			while(i<2)
			{
				BILED01 = i; //set the player to red or green
				BILED00 = !i;
				while(startState == SS) LEDA=1; //wait till slide switch is release if its switched
				LEDA=0;	//set the LED back on
				count=0;
				while(count<336);  //pause for 1 second for a player transistion
		 		//reset errors and turns
				errors=0;
				turns=0;
				while (errors<2 && turns<7) //loop while errors is less than 2
				{			
					inputCase=0;
					oldr = r;   //save the current r as the old value for the next round.
					while (oldr==r) //while the oldr is the same as the current r
						r = random(7); //get random number from 0-6
					count=0;    //reset timer
					Set_outputs();
					while((count*2.9761)<tmax) //loops until time has run out
					{
						//if there is a input case
						if(inputCase!=0)
						{
							//set count to 0
							count = 0;
							while (count<15); //wait 15 for deounce
							inputCase = Check_inputs(inputCase); //reset the new inputCase and break the loop
							break;
						}
						inputCase = Check_inputs(inputCase);
						
					}
					//ensure buzzer is off
					Buzzer=1;
					// if the input case isn't zero and the case matchs
					if(inputCase!=0 && Cases_match(inputCase))
					{	
						//increment turns 
						turns++;
						//set the correct BiLED color to green
						BILED10=0;
						BILED11=1;
						//increment points for a specific player
						if(i==0) pointsr+=5;
						else pointsg+=5;
						//change tmax
						tmax = tmax*.9;
						count=0;
						//wait a second
						while(count<336);
					}else 
					{
						//set the BiLED to red 
						BILED10=1;
						BILED11=0;
						//increment errors
						errors++; 
						count=0;
						//two short 500ms buzzer blasts
						Buzzer=0;
						//do two short buzzer blasts
						while(count<168);
						Buzzer=1;
						count=0;
						while(count<50);
						count=0;
						Buzzer=0;
						while(count<168);
						Buzzer=1;
						//increment the right players errors
						if(i) errorsg=errors;
						else errorsr=errors;
					}
					//reset all the outputs to off
					LEDY=1;
					LEDG=1;
					LEDR=1;
					BILED11=BILED10=1;//clears BILED
					//print the score
					Print_Score(rounds, pointsr, pointsg, errorsr, errorsg);
				}
				//if the errors is 0 (perfect round)
				if(errors==0)
				{
					//award the correct player 6 more points
					if(i==0) pointsr+=6;
					else pointsg+=6;	
				}
				//if the errors is 1 
				if(errors==1)
				{
					//award the correct player 3 more points		
					if(i==0) pointsr+=3;
					else pointsg+=3;	
				}
				//print out the score now
				Print_Score(rounds, pointsr, pointsg, errorsr, errorsg);
				turns=0;
				//change players
				i++;
			}
			//increment the total points for each player by the correct amount
			pointsrt+=pointsr;
			pointsgt+=pointsg;
			//increment the rounds
			rounds++;
		}
		//turn the BiLED off
		BILED01 = 1; 
		BILED00 = 1;
		//print the final score
		printf("\n\rFinal Score:\t\t%i    \t\t\t  %i", pointsrt, pointsgt);
		//print out who won
		if(pointsrt>pointsgt)
		{
			printf("\r\nThe red play wins!");
		}else if(pointsgt>pointsrt)
		{
			printf("\r\nThe green player wins!");
		}else
		{
			printf("\r\nIt is a tie!");
		}
 	}	 
} 

//generates a number number from 0-n and returns it
unsigned char random(unsigned char n)
{
	return (rand()%n);
}
 
//initializes the ports
void Port_Init(void) 
{ 
 	P1MDIN &= ~0x80; // Set P1.7 for analog input 
 	P1MDOUT &= ~0x80; // Set P1.7 to open drain 
 	P1 |= 0x80; // Send logic 1 to input pin P1.7 and 
	P3MDOUT &= ~0x07; // set Port 3 output pins to open drain mode (7 to 3)
    P3MDOUT |= ~0x07; // set Port 3 input pins to push pull mode (0 to 2)
    P3 |= 0x07; // set Port 3 input pins to high impedance state (0 to 2)
	//Port 2
    P2MDOUT |= 0x36; // set Port 2 output pins to open drain mode (5, 4, 2, 1)
	P2MDOUT &= ~0x09; //set port 2 input pins to push pull mode (0, 3)
	P2 |= 0x09;
} 
 
void ADC_Init(void) 
{ 
 	REF0CN = 0x03; // Set Vref to use internal reference voltage (2.4 V) 
 	ADC1CN = 0x80; // Enable A/D converter (ADC1) 
 	ADC1CF |= 0x01; // Set A/D converter gain to 1 
} 
 
unsigned char read_AD_input(unsigned char n) 
{ 
 	AMX1SL = n; // Set P1.n as the analog input for ADC1 
 	ADC1CN = ADC1CN & ~0x20; // Clear the Conversion Completed flag 
 	ADC1CN = ADC1CN | 0x10; // Initiate A/D conversion 
 
 	while ((ADC1CN & 0x20) == 0x00);// Wait for conversion to complete 
 
 	return ADC1; // Return digital value in ADC1 register 
} 

//initializes the interrupts
void Interrupt_Init(void)
{
    IE |= 0x02;      // enable Timer0 Interrupt request
    EA = 1;       // enable global interrupts
}

//initializes the timer
void Timer_Init(void)
{
    CKCON |= 0x08;  // Timer0 uses SYSCLK as source
    TMOD &= 0xF0;   // clear the 4 least significant bits
    TMOD |= 0x01;   // Timer0 in mode 1
    TR0 = 0;           // Stop Timer0
    TL0 = 0;           // Clear low byte of register T0
    TH0 = 0;           // Clear high byte of register T0

}

// function to set outputs
void Set_outputs(void)
{
	char temp;
	//takes the first binary digit of r and sets whether it should be outputed
	LEDG = !(r%2);
	temp = (r/2);
	//takes the second binary digit of r and sets whether it should be outputed	
	LEDR = !(temp%2);
	temp= temp/2;
	//takes the third binary digit of r and sets whether it should be outputed
	LEDY = !(temp%2);
}

//checks if the input case matchs the output case generated by r 
unsigned char Cases_match(unsigned char inputCase)
{
	//if there is a 1 in the first binary digit of the input case the and a 1 in the first binary digit of r, return 0
	if(inputCase%2 && r%2)return 0;
	//if there is a 1 in the second binary digit of the input case the and a 1 in the second binary digit of r, return 0
	if((inputCase/2)%2 && (r/2)%2) return 0;
	//if there is a 1 in the third binary digit of the input case the and a 1 in the third binary digit of r, return 0 
	if(((inputCase/2)/2)%2 && ((r/2)/2)%2) return 0;
	//else the cases are fine return 1
	return 1;
}
//funtction to check inputs and set the input case accordingly
unsigned char Check_inputs(unsigned char inputCase)
{
	//if push button 2 is pressed add 1 the the input case
	if(!PB2)
	{	
		
		inputCase|=0x01;
	}else
	{
		inputCase&=~0x01;
	}
	//if push button 1 is pressed add 2 the the input case
	if(!PB1)
	{
		inputCase|=0x02;
	}else
	{
		inputCase&=~0x02;
	}
	//if push button 0 is pressed add 4 the the input case
	if(!PB0)
	{
		inputCase|=0x04;
	}else
	{
		inputCase&=~0x04;
	}
	return inputCase;
}

//****************
//Prints the current state of the game
void Print_Score(unsigned char rounds, unsigned char pointsr, unsigned char pointsg, unsigned char  errorsr, unsigned char errorsg)
{
	printf("Round: %i of 3 \n\r", rounds+1); //output rounds
	printf("\t\t Red Player Score \t Green Player Score\n\r");
	printf("\rPoints:\t\t        %i  \t\t          %i\n", pointsr, pointsg);//prints out points
	printf("\rErrors (this round):    %i  \t\t          %i\n\r\n\r", errorsr, errorsg);//print errors and skip a line for spacing 
		
}

//***************
void Timer0_ISR(void) __interrupt 1
{
	count++;   // add interrupt code here, in this lab, the code will increment the 
                  // global variable 'counts'
}
