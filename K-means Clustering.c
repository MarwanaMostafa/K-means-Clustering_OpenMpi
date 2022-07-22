#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <omp.h>
#include <unistd.h>

const int numberOfDataPoint = 20;
const int MAXITER = 10;
int CLOSEPROGRAM = 0; // test number of therades =1 and max iterative any number >2 then enter loop only 2 times

typedef struct
{
    float x, y;
    float *ArrayofDistance;
} point;

float mse(point a, point b)
{
    return pow((a.x - b.x), 2) + pow((a.y - b.y), 2);
}

int main()
{
    int i = 0, j = 0;
    int numOfClusters = 0;
#pragma omp parallel
    {
        numOfClusters = omp_get_num_threads();
    }

    point *arrayOfClustersCentroid;
    arrayOfClustersCentroid = (point *)malloc(sizeof(point) * (numOfClusters));

    int pointsToClusters[numberOfDataPoint];

    point *ArrayOfPoints;
    ArrayOfPoints = (point *)malloc(sizeof(point) * (numberOfDataPoint));

    //========================== REEAD FILE==========================

    FILE *ptr;
    ptr = fopen("/shared/DATAINPUT.txt", "r");

    if (NULL == ptr)
        printf("file can't be opened \n");

    float x = 0.0;
    char *token;
    char str[50];

    while (fgets(str, 50, ptr) != NULL && i < numberOfDataPoint)
    {
        token = strtok(str, "  ");
        x = atoi(token);

        ArrayOfPoints[i].x = x;
        token = strtok(NULL, "  ");
        x = atoi(token);
        ArrayOfPoints[i].y = x;
        ArrayOfPoints[i].ArrayofDistance = (float *)malloc(sizeof(float) * (numOfClusters));
        i = i + 1;
    }
    //==========================END REEAD FILE==========================

    //==========================REPEAT FROM STEP 3 TO 5==========================

    int startIterative = 0;
    for (startIterative = 0; startIterative < MAXITER && CLOSEPROGRAM != numberOfDataPoint; startIterative++)
    {

        //==========================Initiate 2 random numbers for each thread/cluster==========================
        if (startIterative == 0)
        {
            for (i = 0; i < numOfClusters; i++)
            {
                arrayOfClustersCentroid[i].x = ArrayOfPoints[i].x;
                arrayOfClustersCentroid[i].y = ArrayOfPoints[i].y;
            }
        }
        //====================================================END Initiated=============================

        //====================================================Calculate the distance between each point andcluster centroid=============================
#pragma omp parallel shared(ArrayOfPoints, arrayOfClustersCentroid, numOfClusters, j) private(i)
        {

#pragma omp for schedule(static)
            for (i = 0; i < numberOfDataPoint; i++)
            {
                for (j = 0; j < numOfClusters; j++)
                {
                    ArrayOfPoints[i].ArrayofDistance[j] = (mse(ArrayOfPoints[i], arrayOfClustersCentroid[j]));
                }
            }
        }
        //==================================ENDDDDDDD Calculate the distance between each point andcluster centroid=============================

        //==========================================Filter each point distances depending on minimum value=============================
        int clusterNumber = 0;
        CLOSEPROGRAM = 0;
        for (i = 0; i < numberOfDataPoint; i++)
        {
            for (j = 1; j < numOfClusters; j++)
            {

                if (ArrayOfPoints[i].ArrayofDistance[clusterNumber] < ArrayOfPoints[i].ArrayofDistance[j])
                    clusterNumber = clusterNumber;
                else
                    clusterNumber = j;
            }
            if (pointsToClusters[i] == clusterNumber)
                CLOSEPROGRAM++;
            pointsToClusters[i] = clusterNumber;
            clusterNumber = 0;
        }

        //==========================================END Filter each point distances depending on minimum value=============================

        //==========================================Calculate the mean for each cluster as new cluster centroid =============================

        for (i = 0; i < numOfClusters; i++)
        {
            arrayOfClustersCentroid[i].x = 0;
            arrayOfClustersCentroid[i].y = 0;
        }

        int x = 0;

        int *NumOfElements;
        NumOfElements = (int *)malloc(sizeof(int) * (numOfClusters));
        for (i = 0; i < numOfClusters; i++)
            NumOfElements[i] = 0;

#pragma omp parallel shared(ArrayOfPoints, arrayOfClustersCentroid) private(i, x)
        {

#pragma omp for schedule(static)
            for (i = 0; i < numberOfDataPoint; i++)
            {
                x = pointsToClusters[i];
                NumOfElements[x]++;
#pragma omp critical
                {
                    arrayOfClustersCentroid[x].x += ArrayOfPoints[i].x;
                    arrayOfClustersCentroid[x].y += ArrayOfPoints[i].y;
                }
                // printf("IN thread %d \n", omp_get_thread_num());
            }
        }

        // i = 0, j = 0;
        // while (i < numOfClusters)
        // {
        // printf("NUM OF ELEMENT FOR INDEX  %d :\n", i + 1);
        // printf("( %d)", NumOfElements[i]);
        // printf("\n");
        // i++;
        // }

// calculate last step in mean : x/size ,y/size for each cluster
#pragma omp parallel shared(arrayOfClustersCentroid, NumOfElements, numOfClusters) private(i)
        {

#pragma omp for schedule(static)
            for (i = 0; i < numOfClusters; i++)
            {
                arrayOfClustersCentroid[i].x /= NumOfElements[i];
                arrayOfClustersCentroid[i].y /= NumOfElements[i];
            }
        }
        //==========================================END Calculate the mean for each cluster as new cluster centroid =============================

        //      i = 0, j = 0;

        // while (i < numOfClusters)
        // {
        //     printf("CLUSTER %d :\n", i + 1);
        //     for (j = 0; j < numberOfDataPoint; j++)
        //     {
        //         if (pointsToClusters[j] == i)
        //         {
        //             printf("( %f ,%f)", ArrayOfPoints[j].x, ArrayOfPoints[j].y);
        //             printf("\n");
        //         }
        //     }
        //     i++;
        // }

    } //==========================END REPEAT FROM STEP 3 TO 5==========================

    //

    //==========================Print OUTPUT ==========================

    i = 0, j = 0;

    while (i < numOfClusters)
    {
        printf("CLUSTER %d :\n", i + 1);
        for (j = 0; j < numberOfDataPoint; j++)
        {
            if (pointsToClusters[j] == i)
            {
                printf("( %f ,%f)", ArrayOfPoints[j].x, ArrayOfPoints[j].y);
                printf("\n");
            }
        }
        i++;
    }

    return 0;
}
