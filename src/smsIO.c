/* 
 * Copyright (c) 2008 MUSIC TECHNOLOGY GROUP (MTG)
 *                         UNIVERSITAT POMPEU FABRA 
 * 
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 */
/*! \file smsIO.c
 * \brief SMS file input and output
 */

#include "sms.h"

/*! \brief file identification constant
 * 
 * constant number that is first within SMS_Header, in order to correctly
 * identify an SMS file when read.  
 */
#define SMS_MAGIC 767  

/*! \brief initialize the header structure of an SMS file
 *
 * \param pSmsHeader	header for SMS file
 */
void sms_initHeader (SMS_Header *pSmsHeader)
{    
	pSmsHeader->iSmsMagic = SMS_MAGIC;
	pSmsHeader->iHeadBSize =  sizeof(SMS_Header);
	pSmsHeader->nFrames = 0;
	pSmsHeader->iFrameBSize = 0;
	pSmsHeader->iFormat = SMS_FORMAT_H;
	pSmsHeader->iFrameRate = 0;
	pSmsHeader->iStochasticType = SMS_STOC_APPROX;
	pSmsHeader->nTracks = 0;
	pSmsHeader->nStochasticCoeff = 0;
	pSmsHeader->fAmplitude = 0;
	pSmsHeader->fFrequency = 0;
	pSmsHeader->iBegSteadyState = 0;
	pSmsHeader->iEndSteadyState = 0;
	pSmsHeader->fResidualPerc = 0;
	pSmsHeader->nLoopRecords = 0;
	pSmsHeader->nSpecEnvelopePoints = 0;
	pSmsHeader->nTextCharacters = 0;
	pSmsHeader->pILoopRecords = NULL;
	pSmsHeader->pFSpectralEnvelope = NULL;
	pSmsHeader->pChTextCharacters = NULL;    
}

/*! \brief fill an SMS header with necessary information for storage
 * 
 * copies parameters from SMS_AnalParams, along with other values
 * so an SMS file can be stored and correctly synthesized at a later
 * time. This is somewhat of a convenience function.
 *
 * \param pSmsHeader    header for SMS file (to be stored)
 * \param nFrames           number of frames in analysis
 * \param pAnalParams   structure of analysis parameters
 * \param iOriginalSRate  samplerate of original input sound file
 * \param nTracks           number of sinusoidal tracks in the analysis
 */
void sms_fillHeader (SMS_Header *pSmsHeader, 
                          int nFrames, SMS_AnalParams *pAnalParams,
                    int iOriginalSRate, int nTracks)
{
        sms_initHeader (pSmsHeader);

        pSmsHeader->nFrames = nFrames;
        pSmsHeader->iFormat = pAnalParams->iFormat;
        pSmsHeader->iFrameRate = pAnalParams->iFrameRate;
        pSmsHeader->iStochasticType = pAnalParams->iStochasticType;
        pSmsHeader->nTracks = nTracks;
	if(pAnalParams->iStochasticType != SMS_STOC_APPROX)
		pSmsHeader->nStochasticCoeff = 0;
        else
                pSmsHeader->nStochasticCoeff = pAnalParams->nStochasticCoeff;
        pSmsHeader->iOriginalSRate = iOriginalSRate;
        pSmsHeader->iFrameBSize = sms_frameSizeB(pSmsHeader);
}

/* void sms_initFrame (SMS_Data *pSmsFrame) */
/* { */
/* 	pSmsFrame->pSmsData = NULL; */
/* 	pSmsFrame->pFSinFreq = NULL; */
/* 	pSmsFrame->pFSinAmp = NULL; */
/* 	pSmsFrame->pFSinPha = NULL; */
/* 	pSmsFrame->pFStocCoeff = NULL; */
/* 	pSmsFrame->pFStocGain = NULL; */
/* 	pSmsFrame->nTracks = 0; */
/* 	pSmsFrame->nCoeff = 0; */
/* 	pSmsFrame->sizeData = 0; */
/* } */


/*! \brief write SMS header to file
 *
 * \param pChFileName	   file name for SMS file
 * \param pSmsHeader header for SMS file
 * \param ppSmsFile     (double pointer to)  file to be created
 * \return error code \see SMS_WRERR in SMS_ERRORS 
 */
int sms_writeHeader (char *pChFileName, SMS_Header *pSmsHeader, 
                    FILE **ppSmsFile)
{
	int iVariableSize = 0;

	if (pSmsHeader->iSmsMagic != SMS_MAGIC)
		return(SMS_NSMS);
	if ((*ppSmsFile = fopen (pChFileName, "w+")) == NULL)
		return(SMS_NOPEN);
	
	/* check variable size of header */
	iVariableSize = sizeof (int) * pSmsHeader->nLoopRecords +
		sizeof (float) * pSmsHeader->nSpecEnvelopePoints +
		sizeof(char) * pSmsHeader->nTextCharacters;

	pSmsHeader->iHeadBSize = sizeof(SMS_Header) + iVariableSize;

	/* write header */
	if (fwrite((void *)pSmsHeader, (size_t)1, (size_t)sizeof(SMS_Header),
	    *ppSmsFile) < (size_t)sizeof(SMS_Header))
                return(SMS_WRERR);
	
	/* write variable part of header */
	if (pSmsHeader->nLoopRecords > 0)
	{
		char *pChStart = (char *) pSmsHeader->pILoopRecords;
		int iSize = sizeof (int) * pSmsHeader->nLoopRecords;
    
		if (fwrite ((void *)pChStart, (size_t)1, (size_t)iSize, *ppSmsFile) < 
		    (size_t)iSize)
                        return(SMS_WRERR);

	}
	if (pSmsHeader->nSpecEnvelopePoints > 0)
	{
		char *pChStart = (char *) pSmsHeader->pFSpectralEnvelope;
		int iSize = sizeof (float) * pSmsHeader->nSpecEnvelopePoints;
    
		if (fwrite ((void *)pChStart, (size_t)1, (size_t)iSize, *ppSmsFile) < 
		    (size_t)iSize)
                        return(SMS_WRERR);
	}
	if (pSmsHeader->nTextCharacters > 0)
	{
		char *pChStart = (char *) pSmsHeader->pChTextCharacters;
		int iSize = sizeof(char) * pSmsHeader->nTextCharacters;
    
		if (fwrite ((void *)pChStart, (size_t)1, (size_t)iSize, *ppSmsFile) < 
		    (size_t)iSize)
                        return(SMS_WRERR);
	}
	return (SMS_OK);
}

/*! \brief rewrite SMS header and close file
 *
 * \param pSmsFile	     pointer to SMS file
 * \param pSmsHeader pointer to header for SMS file
 * \return error code \see SMS_WRERR in SMS_ERRORS 
 */
int sms_writeFile (FILE *pSmsFile, SMS_Header *pSmsHeader)
{
	int iVariableSize;

	rewind(pSmsFile);

	/* check variable size of header */
	iVariableSize = sizeof (int) * pSmsHeader->nLoopRecords +
		sizeof (float) * pSmsHeader->nSpecEnvelopePoints +
		sizeof(char) * pSmsHeader->nTextCharacters;

	pSmsHeader->iHeadBSize = sizeof(SMS_Header) + iVariableSize;

	/* write header */
	if (fwrite((void *)pSmsHeader, (size_t)1, (size_t)sizeof(SMS_Header),
	    pSmsFile) < (size_t)sizeof(SMS_Header))
		return(SMS_WRERR);
	
	/* write variable part of header */
	if (pSmsHeader->nLoopRecords > 0)
	{
		char *pChStart = (char *) pSmsHeader->pILoopRecords;
		int iSize = sizeof (int) * pSmsHeader->nLoopRecords;
    
		if (fwrite ((void *)pChStart, (size_t)1, (size_t)iSize, pSmsFile) < 
		    (size_t)iSize)
			return(SMS_WRERR);
	}
	if (pSmsHeader->nSpecEnvelopePoints > 0)
	{
		char *pChStart = (char *) pSmsHeader->pFSpectralEnvelope;
		int iSize = sizeof (float) * pSmsHeader->nSpecEnvelopePoints;
    
		if (fwrite ((void *)pChStart, (size_t)1, (size_t)iSize, pSmsFile) < 
		    (size_t)iSize)
			return(SMS_WRERR);
	}
	if (pSmsHeader->nTextCharacters > 0)
	{
		char *pChStart = (char *) pSmsHeader->pChTextCharacters;
		int iSize = sizeof(char) * pSmsHeader->nTextCharacters;
    
		if (fwrite ((void *)pChStart, (size_t)1, (size_t)iSize, pSmsFile) < 
		    (size_t)iSize)
			return(SMS_WRERR);
	}

	fclose(pSmsFile);
	return (SMS_OK);
}

/*! \brief write SMS frame
 *
 * \param pSmsFile	        pointer to SMS file
 * \param pSmsHeader  pointer to SMS header
 * \param pSmsFrame   pointer to SMS data frame
 * \return error code \see SMS_WRERR in SMS_ERRORS 
 */
int sms_writeFrame (FILE *pSmsFile, SMS_Header *pSmsHeader, 
                    SMS_Data *pSmsFrame)
{  
	if (fwrite ((void *)pSmsFrame->pSmsData, 1, pSmsHeader->iFrameBSize, 
	            pSmsFile) < pSmsHeader->iFrameBSize)
                return(SMS_WRERR);
	else return (SMS_OK);			
}


/*! \brief get the size in bytes of the frame in a SMS file 
 *
 * \param pSmsHeader    pointer to SMS header
 * \return the size in bytes of the frame
 */
int sms_frameSizeB (SMS_Header *pSmsHeader)
{
	int iSize, nDet;
  
	if (pSmsHeader->iFormat == SMS_FORMAT_H ||
	    pSmsHeader->iFormat == SMS_FORMAT_IH)
		nDet = 2;// freq, mag
        else nDet = 3; // freq, mag, phase

	iSize = sizeof (float) * (nDet * pSmsHeader->nTracks);

        if(pSmsHeader->iStochasticType == SMS_STOC_APPROX)
        {       //stocCoeff + 1 (gain)
                iSize += sizeof(float) * (pSmsHeader->nStochasticCoeff + 1);
        }
        else if(pSmsHeader->iStochasticType == SMS_STOC_IFFT)
        {
                //sizeFFT*2
        }

	return(iSize);
}	     
   

/*! \brief function to read SMS header
 *
 * \param pChFileName		      file name for SMS file
 * \param ppSmsHeader	(double pointer to) SMS header
 * \param ppSmsFile        (double pointer to) inputfile
 * \return error code \see SMS_ERRORS
 */
int sms_getHeader (char *pChFileName, SMS_Header **ppSmsHeader,
                  FILE **ppSmsFile)
{
	int iHeadBSize, iFrameBSize, nFrames;
	int iMagicNumber;
    
	/* open file for reading */
	if ((*ppSmsFile = fopen (pChFileName, "r")) == NULL)
		return (SMS_NOPEN);
            
	/* read magic number */
	if (fread ((void *) &iMagicNumber, (size_t) sizeof(int), (size_t)1, 
	           *ppSmsFile) < (size_t)1)
		return (SMS_RDERR);
	
	if (iMagicNumber != SMS_MAGIC)
		return (SMS_NSMS);

	/* read size of of header */
	if (fread ((void *) &iHeadBSize, (size_t) sizeof(int), (size_t)1, 
		*ppSmsFile) < (size_t)1)
		return (SMS_RDERR);
	
	if (iHeadBSize <= 0)
		return (SMS_RDERR);
     
        /* read number of data Frames */
        if (fread ((void *) &nFrames, (size_t) sizeof(int), (size_t)1, 
                   *ppSmsFile) < (size_t)1)
		return (SMS_RDERR);
        
	if (nFrames <= 0)
		return (SMS_RDERR);
        
        /* read size of data Frames */
	if (fread ((void *) &iFrameBSize, (size_t) sizeof(int), (size_t)1, 
	           *ppSmsFile) < (size_t)1)
		return (SMS_RDERR);
        
	if (iFrameBSize <= 0)
		return (SMS_RDERR);

	/* allocate memory for header */
	if (((*ppSmsHeader) = (SMS_Header *)malloc (iHeadBSize)) == NULL)
		return (SMS_MALLOC);

	/* read header */
	rewind (*ppSmsFile);
	if (fread ((void *) (*ppSmsHeader), 1, iHeadBSize, *ppSmsFile) < iHeadBSize)
		return (SMS_RDERR);

	/* set pointers to variable part of header */
	if ((*ppSmsHeader)->nLoopRecords > 0)
		(*ppSmsHeader)->pILoopRecords = (int *) ((char *)(*ppSmsHeader) + 
			sizeof(SMS_Header));
						
	if ((*ppSmsHeader)->nSpecEnvelopePoints > 0)
		(*ppSmsHeader)->pFSpectralEnvelope = 
			(float *) ((char *)(*ppSmsHeader) + sizeof(SMS_Header) + 
			           sizeof(int) * (*ppSmsHeader)->nLoopRecords);
			
	if ((*ppSmsHeader)->nTextCharacters > 0)
		(*ppSmsHeader)->pChTextCharacters = 
			(char *) ((char *)(*ppSmsHeader) + sizeof(SMS_Header) + 
			sizeof(int) * (*ppSmsHeader)->nLoopRecords +
			sizeof(float) * (*ppSmsHeader)->nSpecEnvelopePoints);

	return (SMS_OK);			
}

/*! \brief read an SMS data frame
 *
 * \param pSmsFile		   pointer to SMS file
 * \param pSmsHeader	   pointer to SMS header
 * \param iFrame               frame number
 * \param pSmsFrame       pointer to SMS frame
 * \return  SMS_OK if it could read the data, -1 if not
 *	
 */
int sms_getFrame (FILE *pSmsFile, SMS_Header *pSmsHeader, int iFrame,
                  SMS_Data *pSmsFrame)
{    
	if (fseek (pSmsFile, pSmsHeader->iHeadBSize + iFrame * 
	                     pSmsHeader->iFrameBSize, SEEK_SET) < 0)
	{
		printf ("sms_getFrame: could not seek to the sms frame %d\n", 
		         iFrame);
		return (-1);
	}
	if ((pSmsHeader->iFrameBSize = 
	       fread ((void *)pSmsFrame->pSmsData, (size_t)1, 
	              (size_t)pSmsHeader->iFrameBSize, pSmsFile))
	    != pSmsHeader->iFrameBSize)
	{
		printf ("sms_getFrame: could not read sms frame %d\n", 
		         iFrame);
		return (-1);
	}
	return (SMS_OK);			
}



/*! \brief  allocate memory for a frame of SMS data
 *
 * \param pSmsFrame	     pointer to a frame of SMS data
 * \param nTracks		      number of sinusoidal tracks in frame
 * \param nCoeff		      number of stochastic coefficients in frame
 * \param iPhase		      whether phase information is in the frame
 * \param stochType           stochastic resynthesis type
 * \return error code \see SMS_MALLOC in SMS_ERRORS
 */
int sms_allocFrame (SMS_Data *pSmsFrame, int nTracks, int nCoeff, int iPhase,
                                       int stochType)
{
        float *dataPos;  /* a marker to locate specific data witin smsData */
	/* calculate size of frame */
	int sizeData = 2 * nTracks * sizeof(float);
        sizeData += 1 * sizeof(float); //adding one for nSamples
	if (iPhase > 0) sizeData += nTracks * sizeof(float);
	if (nCoeff > 0) sizeData += (nCoeff + 1) * sizeof(float);
	/* allocate memory for data */
	if ((pSmsFrame->pSmsData = (float *) malloc (sizeData)) == NULL)
		return (SMS_MALLOC);

	/* set the variables in the structure */
	pSmsFrame->nTracks = nTracks;
	pSmsFrame->nCoeff = nCoeff;
	pSmsFrame->sizeData = sizeData; 
        /* set pointers to data types within smsData array */
	pSmsFrame->pFSinFreq = pSmsFrame->pSmsData;
        dataPos =  (float *)(pSmsFrame->pFSinFreq + nTracks);
	pSmsFrame->pFSinAmp = dataPos;
        dataPos = (float *)(pSmsFrame->pFSinAmp + nTracks);
	if (iPhase > 0)
	{
		pSmsFrame->pFSinPha = dataPos;
                dataPos = (float *) (pSmsFrame->pFSinPha + nTracks);
        }	
	else 	pSmsFrame->pFSinPha = NULL;
	if (nCoeff > 0)
	{
                pSmsFrame->pFStocCoeff = dataPos;
                dataPos = (float *) (pSmsFrame->pFStocCoeff + nCoeff);
                pSmsFrame->pFStocGain = dataPos; 
                dataPos = (float *) (pSmsFrame->pFStocGain + 1);
	}
        else
	{
                pSmsFrame->pFStocCoeff = NULL;
                pSmsFrame->pFStocGain = NULL;
        }
	return (SMS_OK);			
}

/*! \brief  function to allocate an SMS data frame using an SMS_Header
 *
 * this one is used when you have only read the header, such as after 
 * opening a file.
 *
 * \param pSmsHeader	   pointer to SMS header
 * \param pSmsFrame     pointer to SMS frame
 * \return  error code \see SMS_OK and SMS_MALLOC  in SMS_ERRORS 
 */
int sms_allocFrameH (SMS_Header *pSmsHeader, SMS_Data *pSmsFrame)
{
	int iPhase = (pSmsHeader->iFormat == SMS_FORMAT_HP ||
	              pSmsHeader->iFormat == SMS_FORMAT_IHP) ? 1 : 0;
	return (sms_allocFrame (pSmsFrame, pSmsHeader->nTracks, 
                                   pSmsHeader->nStochasticCoeff, iPhase,
                                   pSmsHeader->iStochasticType));
}

/*! \brief free the SMS data structure
 * 
 * \param pSmsFrame	pointer to frame of SMS data
 */
void sms_freeFrame (SMS_Data *pSmsFrame)
{
	free(pSmsFrame->pSmsData);
	pSmsFrame->nTracks = 0;
	pSmsFrame->nCoeff = 0;
	pSmsFrame->sizeData = 0;
	pSmsFrame->pFSinFreq = NULL;
	pSmsFrame->pFSinAmp = NULL;
	pSmsFrame->pFStocCoeff = NULL;
	pSmsFrame->pFStocGain = NULL;
}

/*! \brief clear the SMS data structure
 * 
 * \param pSmsFrame	pointer to frame of SMS data
 */
void sms_clearFrame (SMS_Data *pSmsFrame)
{
	memset ((char *) pSmsFrame->pSmsData, 0, pSmsFrame->sizeData);
}
  
/*! \brief copy a frame of SMS_Data 
 *
 * \param pCopySmsData	copy of frame
 * \param pOriginalSmsData	original frame
 *
 */
void sms_copyFrame (SMS_Data *pCopySmsData, SMS_Data *pOriginalSmsData)
{
	/* if the two frames are the same size just copy data */
	if (pCopySmsData->sizeData == pOriginalSmsData->sizeData &&
	    pCopySmsData->nTracks == pOriginalSmsData->nTracks)
	{
		memcpy ((char *)pCopySmsData->pSmsData, 
	          (char *)pOriginalSmsData->pSmsData,
	          pCopySmsData->sizeData);
	}
	/* if frames of different size copy the smallest */
	else
	{	
		int nTraj = MIN (pCopySmsData->nTracks, pOriginalSmsData->nTracks);
		int nCoeff = MIN (pCopySmsData->nCoeff, pOriginalSmsData->nCoeff);

		pCopySmsData->nTracks = nTraj;
		pCopySmsData->nCoeff = nCoeff;
		memcpy ((char *)pCopySmsData->pFSinFreq, 
	          (char *)pOriginalSmsData->pFSinFreq,
	          sizeof(float) * nTraj);
		memcpy ((char *)pCopySmsData->pFSinAmp, 
	          (char *)pOriginalSmsData->pFSinAmp,
	          sizeof(float) * nTraj);
		if (pOriginalSmsData->pFSinPha != NULL &&
	      pCopySmsData->pFSinPha != NULL)
			memcpy ((char *)pCopySmsData->pFSinPha, 
		          (char *)pOriginalSmsData->pFSinPha,
		          sizeof(float) * nTraj);
		if (pOriginalSmsData->pFStocCoeff != NULL &&
	      pCopySmsData->pFStocCoeff != NULL)
			memcpy ((char *)pCopySmsData->pFStocCoeff, 
	            (char *)pOriginalSmsData->pFStocCoeff,
	            sizeof(float) * nCoeff);
		if (pOriginalSmsData->pFStocGain != NULL &&
	      pCopySmsData->pFStocGain != NULL)
			memcpy ((char *)pCopySmsData->pFStocGain, 
	            (char *)pOriginalSmsData->pFStocGain,
	            sizeof(float));
	}
}

/*! \brief function to interpolate two SMS frames
 *
 * this assumes that the two frames are of the same size
 *
 * \param pSmsFrame1            sms frame 1
 * \param pSmsFrame2            sms frame 2
 * \param pSmsFrameOut        sms output frame
 * \param fInterpFactor              interpolation factor
 */
void sms_interpolateFrames (SMS_Data *pSmsFrame1, SMS_Data *pSmsFrame2,
                           SMS_Data *pSmsFrameOut, float fInterpFactor)
{
	int i;
	float fFreq1, fFreq2;
					 
	/* interpolate the deterministic part */
	for (i = 0; i < pSmsFrame1->nTracks; i++)
	{
		fFreq1 = pSmsFrame1->pFSinFreq[i];
		fFreq2 = pSmsFrame2->pFSinFreq[i];
		if (fFreq1 == 0) fFreq1 = fFreq2;
		if (fFreq2 == 0) fFreq2 = fFreq1;
		pSmsFrameOut->pFSinFreq[i] = 
			fFreq1 + fInterpFactor * (fFreq2 - fFreq1);
		pSmsFrameOut->pFSinAmp[i] = 
			pSmsFrame1->pFSinAmp[i] + fInterpFactor * 
			(pSmsFrame2->pFSinAmp[i] - pSmsFrame1->pFSinAmp[i]);
	}

	/* interpolate the stochastic part. The pointer is non-null when the frame contains
         stochastic coefficients */
	if (pSmsFrameOut->pFStocGain)
        {
                *(pSmsFrameOut->pFStocGain) = 
			*(pSmsFrame1->pFStocGain) + fInterpFactor *
			(*(pSmsFrame2->pFStocGain) - *(pSmsFrame1->pFStocGain));
        }
        for (i = 0; i < pSmsFrame1->nCoeff; i++)
                pSmsFrameOut->pFStocCoeff[i] = 
                        pSmsFrame1->pFStocCoeff[i] + fInterpFactor * 
                        (pSmsFrame2->pFStocCoeff[i] - pSmsFrame1->pFStocCoeff[i]);

}

 
