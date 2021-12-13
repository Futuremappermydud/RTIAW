#pragma once
#include <vector>

int i4vec_frac(int n, float a[], int k)
{
    int frac;
    int i;
    int iryt;
    int j;
    int left;
    int temp;
    int x;
    using std::cerr;

    if (n <= 0)
    {
        cerr << "\n";
        cerr << "I4VEC_FRAC - Fatal error!\n";
        cerr << "  Illegal nonpositive value of N = " << n << "\n";
        exit(1);
    }

    if (k <= 0)
    {
        cerr << "\n";
        cerr << "I4VEC_FRAC - Fatal error!\n";
        cerr << "  Illegal nonpositive value of K = " << k << "\n";
        exit(1);
    }

    if (n < k)
    {
        cerr << "\n";
        cerr << "I4VEC_FRAC - Fatal error!\n";
        cerr << "  Illegal N < K, K = " << k << "\n";
        exit(1);
    }

    left = 1;
    iryt = n;

    for (; ; )
    {
        if (iryt <= left)
        {
            frac = a[k - 1];
            break;
        }

        x = a[k - 1];
        i = left;
        j = iryt;

        for (; ; )
        {
            if (j < i)
            {
                if (j < k)
                {
                    left = i;
                }
                if (k < i)
                {
                    iryt = j;
                }
                break;
            }
            //
            //  Find I so that X <= A(I).
            //
            while (a[i - 1] < x)
            {
                i = i + 1;
            }
            //
            //  Find J so that A(J) <= X.
            //
            while (x < a[j - 1])
            {
                j = j - 1;
            }

            if (i <= j)
            {
                temp = a[i - 1];
                a[i - 1] = a[j - 1];
                a[j - 1] = temp;
                i = i + 1;
                j = j - 1;
            }
        }
    }

    return frac;
}

int i4vec_median(int n, float a[])

{
    int k;
    int median;

    k = (n + 1) / 2;

    median = i4vec_frac(n, a, k);

    return median;
}

float* gray_median_news(int m, int n, float gray[])

{
    float* gray2;
    int i;
    int j;
    float p[5];

    gray2 = new float[m * n];
    //
    //  Process the main part of the image:
    //
    for (i = 1; i < m - 1; i++)
    {
        for (j = 1; j < n - 1; j++)
        {
            p[0] = gray[i - 1 + j * m];
            p[1] = gray[i + 1 + j * m];
            p[2] = gray[i + (j + 1) * m];
            p[3] = gray[i + (j - 1) * m];
            p[4] = gray[i + j * m];

            gray2[i + j * m] = i4vec_median(5, p);
        }
    }
    //
    //  Process the four borders.
    //  Get an odd number of data points, 
    //
    for (i = 1; i < m - 1; i++)
    {
        j = 0;
        p[0] = gray[i - 1 + j * m];
        p[1] = gray[i + 1 + j * m];
        p[2] = gray[i + j * m];
        p[3] = gray[i + (j + 1) * m];
        p[4] = gray[i + (j + 2) * m];
        gray2[i + j * m] = i4vec_median(5, p);

        j = n - 1;
        p[0] = gray[i - 1 + j * m];
        p[1] = gray[i + 1 + j * m];
        p[2] = gray[i + (j - 2) * m];
        p[3] = gray[i + (j - 1) * m];
        p[4] = gray[i + j * m];
        gray2[i + j * m] = i4vec_median(5, p);
    }

    for (j = 1; j < n - 1; j++)
    {
        i = 0;
        p[0] = gray[i + j * m];
        p[1] = gray[i + 1 + j * m];
        p[2] = gray[i + 2 + j * m];
        p[3] = gray[i + (j - 1) * m];
        p[4] = gray[i + (j + 1) * m];
        gray2[i + j * m] = i4vec_median(5, p);

        i = m - 1;
        p[0] = gray[i - 2 + j * m];
        p[1] = gray[i - 1 + j * m];
        p[2] = gray[i + j * m];
        p[3] = gray[i + (j - 1) * m];
        p[4] = gray[i + (j + 1) * m];
        gray2[i + j * m] = i4vec_median(5, p);
    }
    //
    //  Process the four corners.
    //
    i = 0;
    j = 0;
    p[0] = gray[i + 1 + j * m];
    p[1] = gray[i + j * m];
    p[2] = gray[i + (j + 1) * m];
    gray2[i + j * m] = i4vec_median(3, p);

    i = 0;
    j = n - 1;
    p[0] = gray[i + 1 + j * m];
    p[1] = gray[i + j * m];
    p[2] = gray[i + (j - 1) * m];
    gray2[i + j * m] = i4vec_median(3, p);

    i = m - 1;
    j = 0;
    p[0] = gray[i - 1 + j * m];
    p[1] = gray[i + j * m];
    p[2] = gray[i + (j + 1) * m];
    gray2[i + j * m] = i4vec_median(3, p);

    i = m - 1;
    j = n - 1;
    p[0] = gray[i - 1 + j * m];
    p[1] = gray[i + j * m];
    p[2] = gray[i + (j - 1) * m];
    gray2[i + j * m] = i4vec_median(3, p);

    return gray2;
}


std::vector<float> MedianFilter(std::vector<float> input, int height, int width)
{
    int numrows = height, numcols = width;

    float* dest = gray_median_news(numrows, numcols, input.data());

    std::vector<float> dest2;
    for (int i = 0; i < numrows * numcols; i++)
    {
        dest2.push_back(dest[i]);
    }

    std::cout << "D" << std::endl;

    return dest2;
}