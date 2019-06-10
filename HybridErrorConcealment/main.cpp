#include "stdafx.h"
#include <string>

int _tmain(int argc, char* argv[])
{
	int SR=0;	
	int min=0;			
	int max=0;
	std::string temp_str;


	if( argc < 7) 
	{
		printf(" Please Enter Correct Parameter\n");
		return 0;
	}


	temp_str = argv[0];

	SR =atoi(argv[3]);
	min = atoi(argv[4]);
	max = atoi(argv[5]);

	inpainting proposed(argv[1], argv[2]);
	if(proposed.m_bOpen)
		proposed.process(argv[6], SR, min, max);

	//------------------------------------------
	// 결과 출력.
	proposed.SaveResultImage();
	proposed.SaveResultValue("result_process_time.txt");

	printf( "The PSNR is %0.2f\n",proposed.getPSNR());
	printf( "The processing time is %dms\n",proposed.getProcessTime());


	return 0;
}
