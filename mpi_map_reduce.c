#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include<math.h>
#define MASTER_RANK 0
#define TAG_TASK 1
#define TAG_RESULT 2
#define TAG_COMPLETED 3  
#define MAPPED 4
#define REDUCER 5
#define FINAL 6
int const size =16;
int const matrixSize = 4;

void multiplyMatrices(int* row, int** B, int* C, int size)
 {
	//printf("\nMULTIPLYINGG\n");
	for (int i = 0; i < size; i++) 
	{
		int sum=0;
		for (int j = 0; j < size; j++) 
		{
		    sum += row[j] * B[j][i];
		//	printf("ROW %d * Matr%d :%d, ", row[i] ,B[i][j],C[i][j]);
		}
	//printf(":%d, ", sum);
	C[i]=sum;
	}
	//printf("\n");
 }

void writeMatrixToFile(int** final,int size) {
    
    char filename[]="parallel_output.txt";
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        printf("Error opening file: %s\n", filename);
        return;
    }

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            fprintf(file, "%d ", final[i][j]);
        }
        fprintf(file, "\n");
    }
    printf("Output for parallel matrix multiplication stored in parallel_output.txt\n");
    fclose(file);
}

void reduceFunction(int* resultBlock, int*reducedrow, int size,int rank,int redrowsize)
{
	//printf("\nREDUCING %d\n",rank);
	int no_of_rows=resultBlock[0];
	int count=0;int count2=1;

	for (int j=0;j<no_of_rows;j++)
	{
		reducedrow[count]=rank;
		count++;
		reducedrow[count]=j;
		count++;
	    for (int i = count; i < count+size; i++) {
		reducedrow[i] = resultBlock[count2];
		//printf("%d, ",reducedrow[i]);
		count2++;

	    }count+=size;
	}

}

void mapFunction(int* arr1, int** arr2,int*resultrow, int rank,int size)//resultblock should be *
{
    //printf("\nProcess %d in map function\n", rank);
    
    multiplyMatrices(arr1, arr2, resultrow, size);
}

//reading matrix 1
int** readMatrixFromFile(int size) {
    char filename[20];
    snprintf(filename, sizeof(filename), "matrix1_2^%d.txt", size);
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file: %s\n", filename);
        return NULL;
    }
    int msize = pow(2, size);
    int** matrix = (int**)malloc(msize * sizeof(int*));

    for (int i = 0; i < msize; i++) {
        matrix[i] = (int*)malloc(msize * sizeof(int));
    }

    for (int i = 0; i < msize; i++) {
        for (int j = 0; j < msize; j++) {
            if (fscanf(file, "%d", &matrix[i][j]) != 1) {
                printf("Error reading file: %s\n", filename);
                fclose(file);
                return NULL;
            }
        }
    }

    fclose(file);
    return matrix;
}

int** readMatrixFromFile2(int size) {
    char filename[20];
    snprintf(filename, sizeof(filename), "matrix2_2^%d.txt", size);
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file: %s\n", filename);
        return NULL;
    }
    int msize = pow(2, size);
    int** matrix = (int**)malloc(msize * sizeof(int*));
    for (int i = 0; i < msize; i++) {
        matrix[i] = (int*)malloc(msize * sizeof(int));
    }

    for (int i = 0; i < msize; i++) {
        for (int j = 0; j < msize; j++) {
            if (fscanf(file, "%d", &matrix[i][j]) != 1) {
                printf("Error reading file: %s\n", filename);
                fclose(file);
                return NULL;
            }
        }
    }

    fclose(file);
    return matrix;
}
///////////////////////////////
int main(int argc, char** argv)
 {
    int numTasks, rank,name_len;
    char processor_name[MPI_MAX_PROCESSOR_NAME];

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numTasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Get_processor_name(processor_name, &name_len);
   
    int numMappers ;
    int rows=size;
    int numReducers ;
    int blockSize ; 
    
    int** matrix1 = readMatrixFromFile(matrixSize );   //A
    int** matrix2 = readMatrixFromFile2(matrixSize );   //B
    int start =0, end=16 , index = 0;
   // int D1Array[size];
   // int* D1Array1 = (int*)malloc(size * sizeof(int));

	        
        int** finalResultBlock = (int**)malloc(size * sizeof(int*));
        for (int i = 0; i < size; i++) {
            finalResultBlock[i] = (int*)malloc(size * sizeof(int));
            for (int j = 0; j < size; j++) {
                finalResultBlock[i][j] = 0;
            }
        } 

    if (rank == MASTER_RANK) 
    {

	// Flatten the 2D array into a 1D array
	int* flattened = (int*)malloc(size * size * sizeof(int));
	int in = 0;
	for (int i = 0; i < size; i++) {
	    for (int j = 0; j < size; j++) {
		flattened[in++] = matrix2[i][j];
	    }
	}


	MPI_Status status;
	int mapperRank=0;
        printf("Master with process_id %d running on %s\n",rank,processor_name);
       
       //distribution of processes among mappers and reducers

        numReducers = (numTasks-1) / 3; //-1 coz ekfor master
        numMappers = (numTasks-1)-numReducers ;
	printf("No of mappers to reducers : %d:%d\n", numMappers,numReducers);
	int no_of_times=size/numMappers;
	int last = size%numMappers;
	last= no_of_times+last;

        // Assign map tasks to mappers
	//do{
	int* D1Array1 = (int*)malloc(size * no_of_times  * sizeof(int));
	int* D1Array2 = (int*)malloc(size * last * sizeof(int));
        for ( mapperRank = 1; mapperRank <= numMappers; mapperRank++) 
       {    start =0, end=16 ;
            if(mapperRank<numMappers){
		int tempindex=0;
		//printf("%d\n",mapperRank);
		for(int j=index;j<index+no_of_times;j++)
		{
		      for(int i=start ; i<size ; i++)
		      { 
		       D1Array1[tempindex] = matrix1[j][i];
			//printf("%d ",D1Array1[tempindex]);
			tempindex++;
		       }
		      }index+=no_of_times;

            {  

            // Assign a map task to the mapper
            // Send the flattened array
	    MPI_Send(flattened, size * size, MPI_INT, mapperRank, TAG_TASK, MPI_COMM_WORLD);
	  //  printf("sendd 1 %d, mapper%d\n",matrix2[0][1],mapperRank);
            MPI_Send(D1Array1, size*no_of_times, MPI_INT, mapperRank, TAG_RESULT, MPI_COMM_WORLD);
            printf("Task Map assigned to process %d\n", mapperRank);
	//    printf("sendd 2 %d, mapper%d\n",D1Array1[0],mapperRank);

            
             }
					}
            
            else
		{//last process mein all remaining rows
			int tempindex=0;
			//printf("%d\n",mapperRank);
			for(int j=index;j<index+last;j++)
			{
			      for(int i=start ; i<size ; i++)
			      { 
			       D1Array2[tempindex] = matrix1[j][i];
				//printf("%d ",D1Array2[tempindex]);
				tempindex++;
			       }
		      }
			index+=last;

		      
		    // Assign a map task to the mapper
		    // Send the flattened array
		    MPI_Send(flattened, size * size, MPI_INT, mapperRank, TAG_TASK, MPI_COMM_WORLD);
		  //  printf("sendd 1 %d, mapper%d\n",matrix2[0][1],mapperRank);
		    MPI_Send(D1Array2, last*size, MPI_INT, mapperRank, TAG_RESULT, MPI_COMM_WORLD);
		    printf("Task Map assigned to process %d\n", mapperRank);
		  //  printf("sendd 2 %d, mapper%d\n",D1Array2[0],mapperRank);

            
             
		}

       }


	// Broadcast a signal to all processes
        int mapsize2=(size*size)+(numMappers*3);  //size*2 for keys.. keys will be starting row and number of rows
	int* mapper2 = (int*)malloc(mapsize2* sizeof(int));
	int *temp=NULL;
	int mapindex2=0;
        MPI_Bcast(&numMappers, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);
	MPI_Bcast(&numReducers, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);
	

	for ( mapperRank = 1; mapperRank <= numMappers; mapperRank++) 
	{	
		int received_count;
		MPI_Status status;

		// Probe for incoming message size
		MPI_Probe(mapperRank, MAPPED, MPI_COMM_WORLD, &status);

		// Get the size of the received message
		MPI_Get_count(&status, MPI_INT, &received_count);
		temp=(int*)realloc(temp,received_count* sizeof(int));


		MPI_Recv(temp, received_count, MPI_INT, mapperRank, MAPPED, MPI_COMM_WORLD,&status);

		for (int j = 0; j < received_count; j++) 
		    {
			mapper2[mapindex2]=temp[j];
			mapindex2++;
		    }
	}

	    printf("\nMAPPING DONE\n");
	    //MPI_Send(mapper, mapsize, MPI_INT, MASTER_RANK, MAPPED, MPI_COMM_WORLD);
	    for (int j = 0; j < mapsize2; j++) 
	    {
		//printf("%d,",mapper2[j] );
	    }
           // printf("\nMAPPING DONE\n");
//----------------------------------REDUCERS----------------------------------
	int count=0;
	for (int j = 0; j < mapsize2; j++) 
	{
		if(mapper2[j]==-1 )
		    count++;
	}
	//printf("Chunk count:%d\n",count );
	int red_chunk=size/numReducers;
	int lastchunk = size%numReducers;
	lastchunk= red_chunk+lastchunk;
	//printf("Red count:%d\n",red_chunk );
	//printf("Last Chunk count:%d\n",lastchunk );
	int j=0;int total_rows_read=0;int map_count=0;

       // Assign reduce tasks to reducers
        for (int reducerRank = numMappers + 1; reducerRank <= numMappers + numReducers; reducerRank++) 
       {	
		
	
	//printf("\nTOTAL ROWS READ:%d\n",total_rows_read);
            if(reducerRank<numMappers+numReducers)
	    {
		int reducerchunk=(size*red_chunk)+1;  //size*2 for keys.. keys will be starting row and number of rows
		int* reducersend= (int*)malloc(reducerchunk* sizeof(int));
		int no_of_rows=0;int rcount=0;int n=0;int rows_read=0;int counter=0;
		reducersend[rcount]=red_chunk;
		rcount++;
		if(mapper2[j]!=-1 )
		{j=(total_rows_read*size)+(3*map_count);
		int x=j;
		do{
		x--;
		counter++;
		
		}while(mapper2[x]!=-1);
		x++;
		map_count++;
		n=mapper2[x]-(counter/size);
				
		no_of_rows+=mapper2[x];
		
		}

		do
		{

			if(mapper2[j]==-1 )
			    {//printf("\nhere3r1\n");
				j++;
				map_count++;
				n=mapper2[j];
				
				no_of_rows+=mapper2[j];
					//printf("\nrowsssr1%d\n",no_of_rows );
				j++;
				j++;
				}
			if(no_of_rows>red_chunk)
				{
					n=red_chunk-rows_read;
					total_rows_read+=n;
				}
			else
			{
				//n=no_of_rows;
				rows_read+=n;
				total_rows_read=rows_read;
			}
			for (int i=rcount;i<(size*n)+rcount;i++)
			{
				reducersend[i]=mapper2[j];
				//printf("%d,",reducersend[i] );
				j++;
				
				}
			rcount+=(size*n);//printf("\nhere2r1 j%d\n",j);
			

		}while(no_of_rows < red_chunk);
	
	MPI_Send(reducersend, reducerchunk, MPI_INT, reducerRank, REDUCER, MPI_COMM_WORLD);
        printf("Task Reduce assigned to process %d\n", reducerRank);
		}
	    else
	    {
		int reducerchunk=(size*lastchunk)+1;  //size*2 for keys.. keys will be starting row and number of rows
		int* reducersend1= (int*)malloc(reducerchunk* sizeof(int));
		int no_of_rows=0;int rcount=0;int n=0;int rows_read=0;int counter=0;

		reducersend1[rcount]=lastchunk;
		rcount++;
		if(mapper2[j]!=-1 )
		{j=(total_rows_read*size)+(3*map_count);
		int x=j;
		do{
		x--;
		counter++;
		
		}while(mapper2[x]!=-1);
		x++;
		map_count++;
		n=mapper2[x]-(counter/size);
				
		no_of_rows+=mapper2[x];
		
		}
		do
		{

			if(mapper2[j]==-1 )
			    {//printf("\nhere3r1\n");
				j++;
				map_count++;
				n=mapper2[j];
				
				no_of_rows+=mapper2[j];
					//printf("\nrowsssr1%d\n",no_of_rows );
				j++;
				j++;
				}
			if(no_of_rows>lastchunk)
				{
					n=lastchunk-rows_read;
					total_rows_read+=n;
				}
			else
			{
				//n=no_of_rows;
				rows_read+=n;
				total_rows_read=rows_read;
			}
			for (int i=rcount;i<(size*n)+rcount;i++)
			{
				reducersend1[i]=mapper2[j];
				//printf("%d,",reducersend1[i] );
				j++;
				
				}
			rcount+=(size*n);//printf("\nhere2r1 j%d\n",j);
			

		}while(no_of_rows < lastchunk);

	MPI_Send(reducersend1, reducerchunk, MPI_INT, reducerRank, REDUCER, MPI_COMM_WORLD);
        printf("Task Reduce assigned to process %d\n", reducerRank);
	     }

        }

	int mapsize3=(size*size)+(size*2);  //size*2 for keys.. keys will be starting row and number of rows
	int* mapper3 = (int*)malloc(mapsize3* sizeof(int));
	int *temp1=NULL;
	int mapindex3=0;

	for (int reducerRank = numMappers + 1; reducerRank <= numMappers + numReducers; reducerRank++) 
       {
		int received_count1;
		MPI_Status status1;

		// Probe for incoming message size
		MPI_Probe(reducerRank, FINAL, MPI_COMM_WORLD, &status);

		// Get the size of the received message
		MPI_Get_count(&status, MPI_INT, &received_count1);
		temp1=(int*)realloc(temp1,received_count1* sizeof(int));


		MPI_Recv(temp1, received_count1, MPI_INT, reducerRank, FINAL, MPI_COMM_WORLD,&status);

		for (int j = 0; j < received_count1; j++) 
		    {
			mapper3[mapindex3]=temp1[j];
			mapindex3++;
		    }
	}

            printf("\nREDUCING DONE\n");
	
	int** answer = (int**)malloc(size * sizeof(int*));

	for (int i = 0; i < size; i++) 
	{
	     answer[i] = (int*)malloc(size * sizeof(int));
	}
	    int index=0;
	for (int i = 0; i < size; i++) 
	{
		index+=2;
		for (int j = 0; j < size; j++) 
		{
		    answer[i][j] = mapper3[index++];
			
		}
	    }


	//int asnwersize=size*size;
	writeMatrixToFile(answer, size);

	free(flattened);
}


    //Mappers receiving data
    else if (rank != MASTER_RANK )
      {
	//wait for master
	rows=size;
	
        MPI_Bcast(&numMappers, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);
	MPI_Bcast(&numReducers, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);
	
	

	if(rank<=numMappers)
	{

		int mapsize=(size*size)+(size*3);  //size*2 for keys.. keys will be starting row and number of rows
		int* mapper = (int*)malloc(mapsize* sizeof(int));
		int mapindex=0;
		int no_of_times=size/numMappers;
		int last = size%numMappers;
		last= no_of_times+last;
		if(rank<numMappers)
		{
	
		///////////////mappersssss ka kaam
			printf("Process %d received Task Map on %s\n",rank,processor_name);

		      //   printf("%d Reached Receiving function\n",rank);
			   
			    // Assign a map task to the mapper
		  
			   // Receive the flattened array
				int* flattened2 = (int*)malloc(size * size * sizeof(int));
				MPI_Recv(flattened2, size * size, MPI_INT, MASTER_RANK, TAG_TASK, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

				// Create the 2D array and populate it with the received data
				int** arr2 = (int**)malloc(size * sizeof(int*));
				for (int i = 0; i < size; i++) {
				    arr2[i] = (int*)malloc(size * sizeof(int));
				    for (int j = 0; j < size; j++) {
					arr2[i][j] = flattened2[i * size + j];
				    }
				}

			   // printf("Received 1 data :%d, mapper%d\n",arr2[0][1],rank);
			     int* arr1 = (int*)malloc(size *no_of_times* sizeof(int));
				
			     MPI_Recv(arr1, size *no_of_times, MPI_INT, MASTER_RANK, TAG_RESULT, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
		
			     int* arr1choti = (int*)malloc(size *sizeof(int));

			    //printf("Map process %d received data%d\n", rank,arr1[0]);
			

			    //MApfunc(arr1,arr2,rank);
			    int* resrow = (int*)malloc(size * sizeof(int*));
			    int reslast=(size *no_of_times)+3;
			    int* resultBlock = (int*)malloc(reslast* sizeof(int*));
			    int a=0;int b=1;int c=2; int resblock=3;
			    resultBlock[a]=-1;
			    resultBlock[b]=no_of_times;  //key value
	 		    resultBlock[c]=rank; 



				for(int i=0;i<no_of_times;i++)
				{
					//arr1choti mein chunks of arr1
					for (int j = 0; j < size; j++) 
					{
					    arr1choti[j]=arr1[i*size+j];
					    //printf("%d\n",arr1choti[j]);
					}

					//printf("%d YAHAHAHAHAHAHAH\n",no_of_times);
					mapFunction(arr1choti, arr2, resrow,rank, size);
					int res=0;
					for (int j = 3; j < size+3; j++) 
					{
					    resultBlock[i*size+j]=resrow[res];
						res++;
					}
					resblock+=size;
					    //MPI_Send(&(resultBlock[0][0]), size * size, MPI_INT, numMappers+1, TAG_RESULT, MPI_COMM_WORLD);
					//printf("%d ",resultBlock[0]);
				}
			     
			    MPI_Send(resultBlock, reslast, MPI_INT, MASTER_RANK, MAPPED, MPI_COMM_WORLD);
			
			    printf("Process %d completed Task Map on %s\n",rank,processor_name);


			}




//////////////////////////////////LAST WALAY K LIYAY
		else
			{
		       
				int* flattened2 = (int*)malloc(size * size * sizeof(int));
				MPI_Recv(flattened2, size * size, MPI_INT, MASTER_RANK, TAG_TASK, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

				// Create the 2D array and populate it with the received data
				int** arr2 = (int**)malloc(size * sizeof(int*));
				for (int i = 0; i < size; i++) 
				{
				    arr2[i] = (int*)malloc(size * sizeof(int));
				    for (int j = 0; j < size; j++) 
				    {
					arr2[i][j] = flattened2[i * size + j];
				    }
				}

			//    printf("Received 1 data :%d, mapper%d\n",arr2[0][1],rank);
			     int* arr1 = (int*)malloc(size *last* sizeof(int));
				 int* arr1choti = (int*)malloc(size *sizeof(int));

			     MPI_Recv(arr1, size *last, MPI_INT, MASTER_RANK, TAG_RESULT, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
			   // printf("Map process %d received data%d\n", rank,arr1[0]);
		

			    //MApfunc(arr1,arr2,rank);
				int* resrow = (int*)malloc(size * sizeof(int*));
				int reslast=(size *last)+3;
			    	int* resultBlock = (int*)malloc(reslast* sizeof(int*));
				int a=0;int b=1;int c=2;int resblock=3;
				resultBlock[a]=-1;
				resultBlock[b]=last;  //key value
		 		resultBlock[c]=rank; 

				for(int i=0;i<last;i++)
				{
					for (int j = 0; j < size; j++) 
					{
					    arr1choti[j]=arr1[i*size+j];
					    //printf("%d\n",arr1choti[j]);
					}

					//printf("%d YAHAHAHAHAHAHAH\n",no_of_times);
					mapFunction(arr1choti, arr2, resrow,rank, size);
					int res=0;
					for (int j = 3; j < size+3; j++) 
					{
					    resultBlock[i*size+j]=resrow[res];
						res++;
					}
					resblock+=size;
					   
					//printf("%d ",resultBlock[0]);
				}

				
				
				MPI_Send(resultBlock, reslast, MPI_INT, MASTER_RANK, MAPPED, MPI_COMM_WORLD);
				

				//MPI_Send(resultBlock, reslast, MPI_INT, numMappers+1, TAG_RESULT, MPI_COMM_WORLD);
			    printf("Process %d received Task Map on %s\n",rank,processor_name);
			} //MPI_Barrier(MPI_COMM_WORLD); 
             
            
           // else

	}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//REDUCERS
	else if(rank>numMappers)
	{
		
		//printf("numReducers%d\n",numReducers);printf("\nIN REDUCER %d\n",rank);
		// Reducers receiving data
	    
		int red_chunk=size/numReducers;
		int lastchunk = size%numReducers;
		lastchunk= red_chunk+lastchunk;
		

		if(rank<=numMappers + numReducers)
		{

			printf("Process %d received Task Reduce on %s\n",rank,processor_name);
			int chunksize=(size*red_chunk)+1;
			int* mapped = (int*)malloc(chunksize* sizeof(int));
			MPI_Recv(mapped, chunksize, MPI_INT, MASTER_RANK, REDUCER, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
			for (int x = 0; x < chunksize; x++) 
			    {
				//printf("%d,",mapped[x] );
			    }
			int resrows=mapped[0];
			int resrowsize=(size*resrows)+(2*resrows);
			int* resrow1 = (int*)malloc(resrowsize * sizeof(int*));
			

			reduceFunction(mapped, resrow1, size,rank,resrowsize);
			
			
		    	MPI_Send(resrow1, resrowsize, MPI_INT, MASTER_RANK, FINAL, MPI_COMM_WORLD);
			printf("Reducer process %d completed reduce operation\n", rank);
			
	
		}
		else
		{

			printf("Process %d received Task Reduce on %s\n",rank,processor_name);
			int chunksize=(size*lastchunk)+1;
			int* mapped = (int*)malloc(chunksize* sizeof(int));
			MPI_Recv(mapped, chunksize, MPI_INT, MASTER_RANK, REDUCER, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
			for (int x = 0; x < chunksize; x++) 
			    {
				//printf("%d,",mapped[x] );
			    }
			int resrows=mapped[0];
			int resrowsize=(size*resrows)+(2*resrows);
			int* resrow1 = (int*)malloc(resrowsize * sizeof(int*));
			

			reduceFunction(mapped, resrow1, size,rank,resrowsize);
			
		    	MPI_Send(resrow1, resrowsize, MPI_INT, MASTER_RANK, FINAL, MPI_COMM_WORLD);
			printf("Reducer process %d completed reduce operation\n", rank);
			
	
		}
	}

}
MPI_Barrier(MPI_COMM_WORLD); 

	//printf("\nReducer:\n");
    	//for (int i = 0; i < size; i++) {
        
        //for (int j = 0; j < size; j++)
         //   printf("%d ", finalResultBlock[i][j]);
        //printf("\n");
    	//}

	// Clean up memory
        free(finalResultBlock);
          MPI_Finalize();
    return 0;
}
