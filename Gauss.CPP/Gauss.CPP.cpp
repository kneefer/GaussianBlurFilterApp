#include "stdafx.h"
#include "Gauss.CPP.h"

int* ComputePascalRow(int n)
{
	int* row = new int[n + 1];
	row[0] = 1; //First element is always 1
	for (int i = 1; i<n / 2 + 1; i++){ //Progress up, until reaching the middle value
		row[i] = row[i - 1] * (n - i + 1) / i;
	}
	for (int i = n / 2 + 1; i <= n; i++){ //Copy the inverse of the first part
		row[i] = row[n - i];
	}
	return row;
}

void ComputeGaussBlur(ThreadParameters params)
{
	// The pixel array must begin at a memory address that is a multiple of 4 bytes 
	int rowPadded = (params.ImageWidth * 3 + 3) & (~3);

	// Compute difference between rowPadded and real rowWidth
	int rowPaddedDiff = rowPadded - params.ImageWidth * 3;

	int gaussHalf = params.GaussMaskSize / 2;

	BYTE* temp = params.TempImgByteArrayPtr;

	const int gaussWidth = params.GaussMaskSize; // must be odd

	// Compute specific row of a pascal triangle
	int* mask = ComputePascalRow(gaussWidth - 1);

	// Compute gauss mask sum
	int gauss_sum = 0;
	for (int i = 0; i < gaussWidth; i++){
		gauss_sum += mask[i];
	}

	int currPos = 0; // stores position (in bytes) of current position 
	                 // of temporary bitmap array data
	int maxY = params.ImageHeight - gaussWidth + 1;
	BYTE* imgOffset = &params.ImgByteArrayPtr[params.CurrentImgOffset];

	/*
	* Vertical iteration part
	*/

	// Iterate over lines
	for (int y = 0; y < params.ImageHeight; y++)
	{
		int currY = y - gaussHalf;

		// Compute offset to the current line of source bitmap 
		BYTE* offset1 = imgOffset + rowPadded * currY;

		/* 
		* If |current line - gaussHalf| is in bounds of data array
		* (edges of the bitmap)
		*/
		if (currY >= 0 && currY < maxY) 
		{
			// Iterate over pixels in line
			for (int x = 0; x < params.ImageWidth; x++) 
			{
				// Compute offset to the current pixel structure
				BYTE* offset2 = offset1 + x * 3;

				// Clear total sums of R, G, B
				double linc_b = 0;
				double linc_g = 0;
				double linc_r = 0;

				// For each up/down pixel surrounding the current source pixel
				for (int k = 0; k < gaussWidth; k++)
				{
					/*
					* Multiply current mask value by corresponding,
					* surrounding pixel took into consideration of
					* averaging process and add to the total sums
					* splitted on R, G, B components.
					*/
					linc_b += offset2[0] * mask[k];
					linc_g += offset2[1] * mask[k];
					linc_r += offset2[2] * mask[k];

					offset2 += rowPadded;
				}
				
				/*
				* Divide R, G, B components by the gauss mask sum
				* and save averaged value to the temporary data array.
				*/
				temp[currPos++] = linc_b / gauss_sum;
				temp[currPos++] = linc_g / gauss_sum;
				temp[currPos++] = linc_r / gauss_sum;
			}
		}
		/* 
		* If |current line - gaussHalf| is not in bounds of data array
		* (edges of the bitmap)
		*/ 
		else
		{
			// Compute pointer to the current source bitmap pixel structure
			BYTE* offset2 = offset1 + gaussHalf * rowPadded;

			// Iterate over pixels in line
			for (int x = 0; x < params.ImageWidth; x++)
			{
				/*
				* For pixels which are so close to border of the bitmap 
				* that they cannot be averaged by the full mask,
				* we simply rewrite pixel to the temporary data array.
				*/
				temp[currPos++] = offset2[0];
				temp[currPos++] = offset2[1];
				temp[currPos++] = offset2[2];

				offset2 += 3;
			}
		}

		/*
		* Because bitmaps has to have new lines aligned every 4 bytes
		* we have to add row padded difference to make sure we are
		* on the appropriate position of temporary (destination) bitmap
		*/
		currPos += rowPaddedDiff;
	}

	currPos = 0;
	int maxX = params.ImageWidth - gaussWidth + 1;

	int beginCopy = 0;
	int endCopy = params.ImageHeight;

	// If current thread doesn't work with first part of the bitmap
	if (params.IdOfImgPart != 0)
	{
		beginCopy = gaussHalf;
		imgOffset += rowPadded * gaussHalf;
	}

	// If current thread doesn't work with last part of the bitmap
	if (params.IdOfImgPart != params.NumOfImgParts - 1)
		endCopy -= gaussHalf; // shrink area we are working on (because of gauss filters interleaves)

	/*
	* Horizontal iteration part
	*/

	// Iterate over selected range of lines
	for (int y = beginCopy; y<endCopy; y++)
	{
		// Compute offset to the current line of source bitmap 
		BYTE* offset1 = temp + rowPadded * y - gaussHalf * 3;

		// Iterate over pixels in line
		for (int x = 0; x < params.ImageWidth; x++)
		{
			// Clear total sums of R, G, B
			double linc_b = 0;
			double linc_g = 0;
			double linc_r = 0;

			int currX = x - gaussHalf;

			// Compute offset to the current source bitmap pixel structure
			BYTE* offset2 = offset1 + x * 3;

			/*
			* If |current pixel X position - gaussHalf| is in bounds of data array
			* (edges of the bitmap)
			*/
			if (currX >= 0 && currX < maxX)
			{
				// For each left/right pixel surrounding the current source pixel
				for (int k = 0; k < gaussWidth; k++)
				{
					/*
					* Multiply current mask value by corresponding,
					* surrounding pixel took into consideration of
					* averaging process and add to the total sums
					* splitted on R, G, B components.
					*/
					linc_b += offset2[0] * mask[k];
					linc_g += offset2[1] * mask[k];
					linc_r += offset2[2] * mask[k];

					offset2 += 3;
				}

				/*
				* Divide R, G, B components by the gauss mask sum
				* and save averaged value to the temporary data array.
				*/
				imgOffset[currPos++] = linc_b / gauss_sum;
				imgOffset[currPos++] = linc_g / gauss_sum;
				imgOffset[currPos++] = linc_r / gauss_sum;
			}
			/*
			* If |current pixel X position - gaussHalf| is not in bounds of data array
			* (edges of the bitmap)
			*/
			else
			{
				/*
				* For pixels which are so close to border of the bitmap
				* that they cannot be averaged by the full mask,
				* we simply rewrite pixel to the temporary data array.
				*/
				offset2 += gaussHalf * 3;
				imgOffset[currPos++] = offset2[0];
				imgOffset[currPos++] = offset2[1];
				imgOffset[currPos++] = offset2[2];
			}
		}

		/*
		* Because bitmaps has to have new lines aligned every 4 bytes
		* we have to add row padded difference to make sure we are
		* on the appropriate position of temporary (destination) bitmap
		*/
		currPos += rowPaddedDiff;
	}
}