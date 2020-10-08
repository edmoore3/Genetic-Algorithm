/*

Author: Ethan Moore

Date: 02/28/20

Description: This assignment utilizes a genetic algorithm to optimize a theater
with the current COVID-19 health requirements (keeping 6 feet between each person).
It does this by randomly generating seating arangements and allowing the best ones to mutate
and breed.
*/

#include <stdio.h>
#include <time.h>
#include <math.h>

void* malloc(size_t size);
void free(void* ptr);

int rand();
void srand();
int randomInRange(int min, int max);
int shouldMutate(int percentMutated);

int fitnessFunction(int** genome, int numRows, int numSeats, int seatSize, int rowDistance);
void printGenome(int** genome, int numRows, int numSeats);
double distance(int row1, int seat1, int row2, int seat2, int seatSize, int rowDistance);
int fitnessOfSeat(int** genome, int row, int seat, int numRows, int numSeats, int seatSize, int rowDistance);
void mutateGenome(int** genome, int numRows, int numSeats);
int** breedGenomes(const int** genome1, const int** genome2, int numRows, int numSeats);

void sortScores(int* scores, int*** population, int numScores);
void swap1D(int* array, int index1, int index2);
void swap3D(int*** array, int index1, int index2);

double computeAverage(const int* scores, int numScores);

int flushInput();

int main()
{

	// Seed the random function and declare helper variable "extra" for flushing output
	srand(time(0));
	char extra;

	// Declare variables to be used
	int rows, seats, seatSize, rowDistance;
	int numGenomes, numPopulations;
	int percentMutated;
	int*** population;

	// Ask user for how many genomes in each generation
	printf("How many theaters would you like in each generation? ");
	while(((scanf("%d%c", &numGenomes, &extra) != 2 || extra != '\n') && flushInput()) || numGenomes <= 0)
	{
		printf("Please enter a positive integer. ");
	}

	// Ask user for number of rows
	printf("How many rows are in the theater? ");
	while(((scanf("%d%c", &rows, &extra) != 2 || extra != '\n') && flushInput()) || rows <= 0)
	{
		printf("Please enter a positive integer. ");
	}

	// Ask user for number of seats per row
	printf("How many seats are in each row? ");
	while(((scanf("%d%c", &seats, &extra) != 2 || extra != '\n') && flushInput()) || seats <= 0)
	{
		printf("Please enter a positive integer. ");
	}

	// Ask user for size of seat
	printf("What is the size of each seat (in inches)? ");
	while(((scanf("%d%c", &seatSize, &extra) != 2 || extra != '\n') && flushInput()) || seatSize <= 0)
	{
		printf("Please enter a positive integer. ");
	}

	// Ask user for distance between each row
	printf("What is the distance between each row (in inches)? ");
	while(((scanf("%d%c", &rowDistance, &extra) != 2 || extra != '\n') && flushInput()) || rowDistance <= 0)
	{
		printf("Please enter a positive integer. ");
	}

	// Ask user how many times they would like to repopulate
	printf("How many generations would you like to create? ");
	while(((scanf("%d%c", &numPopulations, &extra) != 2 || extra != '\n') && flushInput()) || numPopulations <= 0)
	{
		printf("Please enter a positive integer. ");
	}

	printf("Please enter a percent chance between 5 and 20 that a surviving genome mutates.\n"
		"Enter a percent as a number (I.e. 5 percent would be '5'). ");
	while(((scanf("%d%c", &percentMutated, &extra) != 2 || extra != '\n') && flushInput()) || percentMutated < 5 || percentMutated > 20)
	{
		printf("Please enter an integer between 5 and 20. ");
	}

	// Initialize the array of genomes to random values
	int genome, rowNumber, seatNumber;
	population = (int***) malloc(numGenomes * sizeof(int**));
	for(genome = 0; genome < numGenomes; genome++)
	{
		population[genome] = (int**) malloc(rows * sizeof(int*));
		for(rowNumber = 0; rowNumber < rows; rowNumber++)
		{
			population[genome][rowNumber] = (int*) malloc(seats * sizeof(int));
			for(seatNumber = 0; seatNumber < seats; seatNumber++)
			{
				population[genome][rowNumber][seatNumber] = (rand() % 2);
			}
		}
	}

	// Scores will be a parallel array to represent the scores of the genomes
	int* scores = (int*) malloc(numGenomes * sizeof(int));
	do
	{
		// Main loop, runs once for every generation
		while(numPopulations != 0)
		{
			int genome, rowNumber, seatNumber;
			// Score each genome and save the score into the parallel int array
			for(genome = 0; genome < numGenomes; genome++)
			{
				scores[genome] = fitnessFunction(population[genome], rows, seats, seatSize, rowDistance);
			}

			// Sort the arrays and print them all out
			sortScores(scores, population, numGenomes);
			for(genome = 0; genome < numGenomes; genome++)
			{
				printGenome(population[genome], rows, seats);
				printf("Score: %d\n"
					"***************\n\n", scores[genome]);
			}

			printf("Average of this population = %f\n", computeAverage(scores, numGenomes));
			if(numPopulations != 1)
			{
				printf("Press enter to generate next population.\n"
					"You can hold enter to continuously run many populations.");
			 flushInput();
			}

			for(genome = 0; genome < numGenomes/2; genome++)
			{
				for(rowNumber = 0; rowNumber < rows; rowNumber++)
				{
					for(seatNumber = 0; seatNumber < seats; seatNumber++)
					{
						population[genome][rowNumber][seatNumber] = 0;
					}
				}
			}

			// If at local max, increase percent mutated to avoid getting stuck at local max
			if(scores[numGenomes-1] == scores[numGenomes/2] && percentMutated < 20)
			{
				percentMutated++;
			}

			// Possibility to mutate top half of genomes
			for(genome = numGenomes/2; genome < numGenomes; genome++)
			{
				// If this genome should be mutated, store mutated genome in highest low scoring
				// genome.
				if(shouldMutate(percentMutated))
				{
					mutateGenome(population[genome], rows, seats);
					scores[genome] = fitnessFunction(population[genome], rows, seats, seatSize, rowDistance);
				}
			}

			// Breed upper half of genomes (not including mutated ones) to fill empty slots
			for(genome = 0; genome < numGenomes/2; genome++)
			{
				int genome1 = randomInRange(numGenomes/2, numGenomes);
				int genome2 = randomInRange(numGenomes/2, numGenomes);
				while(genome1 == genome2 && numGenomes > 2)
				{
					genome2 = randomInRange(numGenomes/2, numGenomes);
				}
				population[genome] = breedGenomes((const int**)population[genome1], (const int**)population[genome2], rows, seats);
			}

			numPopulations--;
		}

		// Give the user a choice to continue running the simulation
		printf("How many more generations would you like to run?\n"
			"Enter 0 if you would like to finish the simulation. ");
		while(((scanf("%d%c", &numPopulations, &extra) != 2 || extra != '\n') && flushInput()) || numPopulations < 0)
		{
			printf("Please enter a non negative integer. ");
		}
	} while(numPopulations != 0);

	// Free allocated memory
	free(scores);
	for(genome = 0; genome < numGenomes; genome++)
	{
		for(rowNumber = 0; rowNumber < rows; rowNumber++)
		{
			free(population[genome][rowNumber]);
		}
		free(population[genome]);
	}
	free(population);
}

/*	flushInput()
	Basic function to flush and validate user input
*/
int flushInput()
{
	while(getchar() != '\n');
	return 1;
}

/* 	distance()
	This function is used to find the distance between the middle of two seats in the theater.
	Parameters:	row1 - the row of the first seat
				seat1 - the index of the first seat in its row
				row2 - the row of the second seat
				seat2 - the index of the second seat in its row
				seatSize - the size of the seats in the theater (center to center)
				rowDistance - the distance between rows in the theater (center of seat to center of seat)
	Returns:	Distance - the distance between the two given seats
*/
double distance(int row1, int seat1, int row2, int seat2, int seatSize, int rowDistance)
{
	// First convert to feet
	double sizeInFeet = seatSize/12;
	double rowDistInFeet = rowDistance/12;
	// Then find the difference in the x and y directions
	int dx = fabs(row2 - row1);
	int dy = fabs(seat2 - seat1);
	double deltax = dx*sizeInFeet;
	double deltay = dy*rowDistInFeet;
	double distance = sqrt(deltax*deltax + deltay*deltay);
	return distance;
}

/*	computeAverage()
	This function is used to compute the average score for a population.
	Parameters:	scores - the array of scores for a given population
				numScores - the length of the array
	Returns:	average - the average value of the array
*/
double computeAverage(const int* scores, int numScores)
{
	double average = 0.0;	// Variable to be returned
	int score;	// Iterator variable
	for(score = 0; score < numScores; score++)
	{
		average += scores[score];
	}
	average /= numScores;
	return average;
}

/*	printGenome()
	This function prints out a theater.
	Parameters:	genome - the 2D array to be printed
				numRows - the number of rows in the theater
				numSeats - the number of seats in each row
*/
void printGenome(int** genome, int numRows, int numSeats)
{
	int rowNumber, seatNumber;	// Iterator variables
	for(rowNumber = 0; rowNumber < numRows; rowNumber++)
	{
		for(seatNumber = 0; seatNumber < numSeats; seatNumber++)
		{
			printf("%d ", genome[rowNumber][seatNumber]);
		}
		printf("\n");
	}
}

/*	fitnessFunction()
	This function grades a theater. It uses the same fitness function outlined in the project description:
		+1 for every person in the theater
		-2 for every CDC violation in the theater
		+10 if there are no CDC violations
	Parameters:	genome - the theater to be graded
				numRows - the number of rows in the theater
				numSeats - the number of seats in each row
				seatSize - the size of each seat (center to center)
				rowDistance - the distance between rows (center of seat to center of seat)
	Returns:	score - the score of the theater according to the outlined fitness function
*/
int fitnessFunction(int** genome, int numRows, int numSeats, int seatSize, int rowDistance)
{
	int isSafe = 1;		// To check if there are no CDC violations
	int rowNumber, seatNumber;	// Iterator variables
	int seatFitness = 0;	// Used to judge an individual seat
	int score = 0;		// The variable to be returned
	for(rowNumber = 0; rowNumber < numRows; rowNumber++)
	{
		for(seatNumber = 0; seatNumber < numSeats; seatNumber++)
		{
			if(genome[rowNumber][seatNumber] == 1)
			{
				score++;	// Increment score if seat is full
				seatFitness = fitnessOfSeat(genome, rowNumber, seatNumber, numRows, numSeats, seatSize, rowDistance);
				score -= seatFitness;	// Subtract 2 for every CDC violation (clculated using fitnessOfSeat)
				if(seatFitness != 0)
				{
					isSafe = 0;	// If any seat has a CDC violation, the theater is not safe
				}
			}
		}
	}
	if(isSafe)
	{
		score += 10;
	}
	return score;
}

/*	fitnessOfSeat()
	This is a helper function for the fitness function that checks the fitness of a specific seat. To avoid double counting, each seat checks only
	the seats to the right of it in its own row and all the seats below it
	Parameters:	genome - the theater that the seat belongs to
				row - the row the seat belongs to
				seat - the position of the seat in its row
				numRows - the number of rows in the theater
				numSeats - the number of seats in each row
				seatSize - the size of each seat (center to center)
				rowDistance - the distance between each row (center of seat to center of seat)
	Returns:	numToSubtract - equal to 2*(# of CDC violations), it represents the number that will be taken away from the total for the theater
*/
int fitnessOfSeat(int** genome, int row, int seat, int numRows, int numSeats, int seatSize, int rowDistance)
{
	int rowToCheck, seatToCheck;	// Iterator variables
	int numToSubtract = 0;	// The variable to be returned
	for(rowToCheck = row; rowToCheck < numRows; rowToCheck++)
	{
		int startingSeat = 0;	// Starting seat is set to 0 by default (meaning the whole row will be checked)
		if(rowToCheck == row)
		{
			startingSeat = seat;	// If currently in the first row, only check to the right of the seat
		}
		for(seatToCheck = startingSeat; seatToCheck < numSeats; seatToCheck++)
		{
			if(genome[rowToCheck][seatToCheck] == 1 && (rowToCheck != row || seatToCheck != seat))
			{
				if(distance(row, seat, rowToCheck, seatToCheck, seatSize, rowDistance) < 6)
				{
					numToSubtract += 2;	// If a seat less than 6 feet away is also occupied, add 2 to the number to be subtracted
				}
			}
		}
	}
	return numToSubtract;
}

/*	sortScores()
	This function sorts the array of genomes by score by using a parallel array of scores utilizing a bubble sort.
	Parameters:	scores - the array of scores
				population - the array of genomes
				numScores - the leangth of the arrays
*/
void sortScores(int* scores, int*** population, int numScores)
{
	int index;
	for(index = 0; index < numScores - 1; index++)
	{
		int idxToCompare;
		for(idxToCompare = 0; idxToCompare < numScores - index - 1; idxToCompare++)
		{
			if(scores[idxToCompare] > scores[idxToCompare+1])
			{
				swap1D(scores, idxToCompare, idxToCompare+1);
				swap3D(population, idxToCompare, idxToCompare+1);
			}
		}
	}
}

/*	swap1D()
	A helper function to swap the elements of scores as needed.
	Parameters:	array - the 1D array to swap 2 elements of
				index1 - the index of the first element
				index2 - the index of the second element
*/
void swap1D(int* array, int index1, int index2)
{
	int temp = array[index1];
	array[index1] = array[index2];
	array[index2] = temp;
}

/*	swap3D
	A helper function to swap the elements of population as needed.
	Parameters:	array - the 3D array to swap elements of
				index1 - the index of the first element
				index2 - the index of the second element
*/
void swap3D(int*** array, int index1, int index2)
{
	int** temp = array[index1];
	array[index1] = array[index2];
	array[index2] = temp;
}

/*	randomInRange()
	A helper function that returns aa random integer between two numbers.
	Parameters:	min - the minimum possible value for the random integer
				max - the maximum possible value for the random integer (non inclusive)
	Returns:	random - a random integer in the range [min, max)
*/
int randomInRange(int min, int max)
{ 
	int range = max - min;
	int zeroToRange = rand() % range;
	int random = zeroToRange + min;
	return random;
}

/*	shouldMutate()
	Helper method that decides whether or not to mutate a genome.
	Parameters:	percentMutated - the percent chance that a genome is mutated
	Returns:	1 if the genome should be mutated, 0 otherwise
*/
int shouldMutate(int percentMutated)
{
	int shouldMutate = 0;
	if(rand()%100 < percentMutated)
	{
		shouldMutate = 1;
	}
	return shouldMutate;
}

/*	mutateGenome()
	This function mutates a genome in the population
	Parameters:	genome - the genome to be mutated
				rows - the number of rows in the theater
				seats - the number of seats in each row
*/
void mutateGenome(int** genome, int rows, int seats)
{
	int rowToCheck, seatToCheck;
	int percentToChange = randomInRange(1, 11);
	for(rowToCheck = 0; rowToCheck < rows; rowToCheck++)
	{
		for(seatToCheck = 0; seatToCheck < seats; seatToCheck++)
		{
			if(shouldMutate(percentToChange))
			{
				genome[rowToCheck][seatToCheck] = (genome[rowToCheck][seatToCheck] + 1) % 2;
			}
		}
	}
}

/*	breedGenomes()
	This function breeds two genomes to make a new genome
	Parameters:	genome1 - the first genome
				genome2 - the second genome
				numRows - the number of rows in the theater
				numSeats - the number of seats in each row
	Returns:	bredGenome - the genome that results from the combination of the two
*/
int** breedGenomes(const int** genome1, const int** genome2, int numRows, int numSeats)
{
	int** bredGenome = (int**) malloc(numRows * sizeof(int*));
	int rowToCheck, seatToCheck;
	for(rowToCheck = 0; rowToCheck < numRows; rowToCheck++)
	{
		bredGenome[rowToCheck] = (int*) malloc(numSeats * sizeof(int));
		int fromGenome1 = randomInRange(0,2);
		int seatsFromGenome1 = randomInRange(numSeats/3,(numSeats*2)/3);
		if(fromGenome1)
		{
			for(seatToCheck = 0; seatToCheck < seatsFromGenome1; seatToCheck++)
			{
				bredGenome[rowToCheck][seatToCheck] = genome1[rowToCheck][seatToCheck];
			}
			for(seatToCheck = seatsFromGenome1; seatToCheck < numSeats; seatToCheck++)
			{
				bredGenome[rowToCheck][seatToCheck] = genome2[rowToCheck][seatToCheck];
			}
		}
		else
		{
			for(seatToCheck = 0; seatToCheck < seatsFromGenome1; seatToCheck++)
			{
				bredGenome[rowToCheck][seatToCheck] = genome2[rowToCheck][seatToCheck];
			}
			for(seatToCheck = seatsFromGenome1; seatToCheck < numSeats; seatToCheck++)
			{
				bredGenome[rowToCheck][seatToCheck] = genome1[rowToCheck][seatToCheck];
			}
		}
	}
	return bredGenome;
}