#include <string>
#include <iostream>
#include <fstream>
#include <time.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/ml/ml.hpp>

using namespace std;
using namespace cv;
using namespace cv::ml;

#define PosSamNO 2400  //����������
#define NegSamNO 12000 //����������

#define TRAIN false   //�Ƿ����ѵ��,true��ʾ����ѵ����false��ʾ��ȡxml�ļ��е�SVMģ��
#define CENTRAL_CROP true   //true:ѵ��ʱ����96*160��INRIA������ͼƬ���ó��м��64*128��С����

//HardExample�����������������HardExampleNO����0����ʾ�������ʼ���������󣬼�������HardExample����������
//��ʹ��HardExampleʱ��������Ϊ0����Ϊ������������������������ά����ʼ��ʱ�õ����ֵ
#define HardExampleNO 0  

int main()
{
	//��ⴰ��(64,128),��ߴ�(16,16),�鲽��(8,8),cell�ߴ�(8,8),ֱ��ͼbin����9
	HOGDescriptor hog(Size(64,128),Size(16,16),Size(8,8),Size(8,8),9);//HOG���������������HOG�����ӵ�
	int DescriptorDim;//HOG�����ӵ�ά������ͼƬ��С����ⴰ�ڴ�С�����С��ϸ����Ԫ��ֱ��ͼbin��������
	Ptr<SVM> svm = SVM::create(); // SVM������

	//��TRAINΪtrue������ѵ��������
	if(TRAIN)
	{
		string ImgName;//ͼƬ��(����·��)
		ifstream finPos("INRIAPerson96X160PosList.txt");//������ͼƬ���ļ����б�
		//ifstream finPos("PersonFromVOC2012List.txt");//������ͼƬ���ļ����б�
		ifstream finNeg("NoPersonFromINRIAList.txt");//������ͼƬ���ļ����б�

		Mat sampleFeatureMat;//����ѵ������������������ɵľ��������������������ĸ�������������HOG������ά��	
		Mat sampleLabelMat;//ѵ����������������������������������ĸ�������������1��1��ʾ���ˣ�-1��ʾ����


		//���ζ�ȡ������ͼƬ������HOG������
		for(int num=0; num<PosSamNO && getline(finPos,ImgName); num++)
		{
			cout<<"����"<<ImgName<<endl;
			ImgName = "C:\\Users\\zzy19\\Documents\\GitHub\\INRIAPerson\\96X160H96\\Train\\pos\\" + ImgName;//������������·����
			Mat src = imread(ImgName);//��ȡͼƬ
			if(CENTRAL_CROP)
				src = src(Rect(16,16,64,128));//��96*160��INRIA������ͼƬ����Ϊ64*128������ȥ�������Ҹ�16������
			//resize(src,src,Size(64,128));

			vector<float> descriptors;//HOG����������
			hog.compute(src,descriptors,Size(8,8));//����HOG�����ӣ���ⴰ���ƶ�����(8,8)
			//
			cout<<"������ά����"<<descriptors.size()<<endl;

			//�����һ������ʱ��ʼ�����������������������Ϊֻ��֪��������������ά�����ܳ�ʼ��������������
			if( 0 == num )
			{
				DescriptorDim = descriptors.size();//HOG�����ӵ�ά��
				//��ʼ������ѵ������������������ɵľ��������������������ĸ�������������HOG������ά��sampleFeatureMat
				sampleFeatureMat = Mat::zeros(PosSamNO + NegSamNO + HardExampleNO, DescriptorDim, CV_32FC1);
				//��ʼ��ѵ����������������������������������ĸ�������������1��1��ʾ���ˣ�0��ʾ����
				sampleLabelMat = Mat::zeros(PosSamNO + NegSamNO + HardExampleNO, 1, CV_32SC1);
			}

			//������õ�HOG�����Ӹ��Ƶ�������������sampleFeatureMat
			for(int i=0; i<DescriptorDim; i++)
				sampleFeatureMat.at<float>(num,i) = descriptors[i];//��num�����������������еĵ�i��Ԫ��
			sampleLabelMat.at<int>(num,0) = 1;//���������Ϊ1������
		}

		//���ζ�ȡ������ͼƬ������HOG������
		for(int num=0; num<NegSamNO && getline(finNeg,ImgName); num++)
		{
			cout<<"����"<<ImgName<<endl;
			ImgName = "C:\\Users\\zzy19\\Documents\\GitHub\\INRIAPerson\\negphoto\\" + ImgName;//���ϸ�������·����
			Mat src = imread(ImgName);//��ȡͼƬ
			//resize(src,img,Size(64,128));

			vector<float> descriptors;//HOG����������
			hog.compute(src,descriptors,Size(8,8));//����HOG�����ӣ���ⴰ���ƶ�����(8,8)
			//cout<<"������ά����"<<descriptors.size()<<endl;

			//������õ�HOG�����Ӹ��Ƶ�������������sampleFeatureMat
			for(int i=0; i<DescriptorDim; i++)
				sampleFeatureMat.at<float>(num+PosSamNO,i) = descriptors[i];//��PosSamNO+num�����������������еĵ�i��Ԫ��
			sampleLabelMat.at<int>(num+PosSamNO,0) = -1;//���������Ϊ-1������
			
		}

		//����HardExample������
		if(HardExampleNO > 0)
		{
			ifstream finHardExample("HardExample_2400PosINRIA_12000NegList.txt");//HardExample���������ļ����б�
			//���ζ�ȡHardExample������ͼƬ������HOG������
			for(int num=0; num<HardExampleNO && getline(finHardExample,ImgName); num++)
			{
				cout<<"����"<<ImgName<<endl;
				ImgName = "D:\\DataSet\\HardExample_2400PosINRIA_12000Neg\\" + ImgName;//����HardExample��������·����
				Mat src = imread(ImgName);//��ȡͼƬ
				//resize(src,img,Size(64,128));

				vector<float> descriptors;//HOG����������
				hog.compute(src,descriptors,Size(8,8));//����HOG�����ӣ���ⴰ���ƶ�����(8,8)
				//cout<<"������ά����"<<descriptors.size()<<endl;

				//������õ�HOG�����Ӹ��Ƶ�������������sampleFeatureMat
				for(int i=0; i<DescriptorDim; i++)
					sampleFeatureMat.at<float>(num+PosSamNO+NegSamNO,i) = descriptors[i];//��PosSamNO+num�����������������еĵ�i��Ԫ��
				sampleLabelMat.at<int>(num+PosSamNO+NegSamNO,0) = -1;//���������Ϊ-1������
			}
		}

		//���������HOG�������������ļ�
	/*	ofstream fout("SampleFeatureMat.txt");
		for(int i=0; i<PosSamNO+NegSamNO; i++)
		{
			fout<<i<<endl;
			for(int j=0; j<DescriptorDim; j++)
			{	fout<<sampleFeatureMat.at<float>(i,j)<<"  ";
	              		
			}
			fout<<endl;
		}*/

		//ѵ��SVM������
		//SVM������SVM����ΪC_SVC�����Ժ˺������ɳ�����C=0.01
		svm->setType(SVM::Types::C_SVC);
		svm->setKernel(SVM::KernelTypes::LINEAR);
		//svm->setDegree(0);
		//svm->setGamma(1);
		//svm->setCoef0(0);
		//svm->setNu(0);
		//svm->setP(0);
		svm->setC(0.01);

		//������ֹ��������������1000�λ����С��FLT_EPSILONʱֹͣ����
		svm->setTermCriteria(TermCriteria(CV_TERMCRIT_ITER + CV_TERMCRIT_EPS, 1000, FLT_EPSILON));

		cout<<"��ʼѵ��SVM������"<<endl;
		svm->train(sampleFeatureMat, SampleTypes::ROW_SAMPLE, sampleLabelMat);
		cout<<"ѵ�����"<<endl;
		svm->save("SVM_HOG_1500PosINRIA_2000Neg(̫����).xml");//��ѵ���õ�SVMģ�ͱ���Ϊxml�ļ�

	}
	else //��TRAINΪfalse����XML�ļ���ȡѵ���õķ�����
	{
		svm = StatModel::load<SVM>("SVM_HOG.xml");//��XML�ļ���ȡѵ���õ�SVMģ��
		//Ptr<SVM> svm = SVM::load<SVM>("SVM_HOG.xml");
	}


	/*************************************************************************************************
	����SVMѵ����ɺ�õ���XML�ļ����棬��һ�����飬����support vector������һ�����飬����alpha,��һ��������������rho;
	��alpha����ͬsupport vector��ˣ�ע�⣬alpha*supportVector,���õ�һ����������֮���ٸ���������������һ��Ԫ��rho��
	��ˣ���õ���һ�������������ø÷�������ֱ���滻opencv�����˼��Ĭ�ϵ��Ǹ���������cv::HOGDescriptor::setSVMDetector()����
	�Ϳ����������ѵ������ѵ�������ķ������������˼���ˡ�
	***************************************************************************************************/
	DescriptorDim = svm->getVarCount();//����������ά������HOG�����ӵ�ά��
	Mat supportVectorMat = svm->getSupportVectors();//supportVectorMatԪ�ص���������Ϊfloat
	const int supportVectorNum = supportVectorMat.rows;//֧�������ĸ���
	cout<<"֧������������"<<supportVectorNum<<endl;
	Mat alphaMat, supportVectorIndex;
	alphaMat = Mat::zeros(supportVectorNum, DescriptorDim, CV_32F);//alphamat��svindex�����ʼ��������getDecisionFunction()�����ᱨ��
	supportVectorIndex = Mat::zeros(1, supportVectorNum, CV_64F);
	//alpha���������ȵ���֧����������
	//supportVectorIndex,֧����������
	Mat resultMat;//alpha��������֧����������Ľ��
	
	double rho = svm->getDecisionFunction(0, alphaMat, supportVectorIndex);
	alphaMat.convertTo(alphaMat, CV_32F);//��alphamatԪ�ص�������������ת��CV_32F
	
	//����-(alphaMat * supportVectorMat),����ŵ�resultMat��
	//gemm(alphaMat, supportVectorMat, -1, 0, 1, resultMat);//��֪��Ϊʲô�Ӹ��ţ�
	resultMat = -1 * alphaMat * supportVectorMat;

	//�õ����յ�setSVMDetector(const vector<float>& detector)�����п��õļ����
	vector<float> myDetector;
	//��resultMat�е����ݸ��Ƶ�����myDetector��
	for(int i=0; i<DescriptorDim; i++)
	{
		myDetector.push_back(resultMat.at<float>(0,i));
	}
	//������ƫ����rho���õ������
	myDetector.push_back(rho);
	cout<<"�����ά����"<<myDetector.size()<<endl;

	//����HOGDescriptor�ļ����
	HOGDescriptor myHOG;
	myHOG.setSVMDetector(myDetector);
	//myHOG.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector());

	//�������Ӳ������ļ�
	ofstream fout("HOGDetectorForOpenCV.txt");
	for(int i=0; i<myDetector.size(); i++)
	{
		fout<<myDetector[i]<<endl;
	}

	/**************����ͼƬ����HOG���˼��******************/
	//Mat src = imread("00000.jpg");
	Mat src = imread("2007_000423.jpg");
	//Mat src = imread("noperson000001.jpg");
	vector<Rect> found, found_filtered;//���ο�����
	cout<<"���ж�߶�HOG������"<<endl;

	clock_t start, finish;
	double TheTimes;
	start = clock();
	myHOG.detectMultiScale(src, found, 0, Size(8,8), Size(32,32), 1.05, 2);//��ͼƬ���ж�߶����˼��
	finish = clock();
	TheTimes = finish - start;
	printf("%d����\n", int(TheTimes));

	 //srcΪ���������ͼƬ��foundΪ��⵽Ŀ�������б�����3Ϊ�����ڲ�����Ϊ����Ŀ�����ֵ��Ҳ���Ǽ�⵽��������SVM���೬ƽ��ľ���;
    //����4Ϊ��������ÿ���ƶ��ľ��롣�������ǿ��ƶ���������������5Ϊͼ������Ĵ�С������6Ϊ����ϵ����������ͼƬÿ�γߴ��������ӵı�����
    //����7Ϊ����ֵ����У��ϵ������һ��Ŀ�걻������ڼ�����ʱ���ò�����ʱ�����˵������ã�Ϊ0ʱ��ʾ����������á�
	cout<<"�ҵ��ľ��ο������"<<found.size()<<endl;

	//�ҳ�����û��Ƕ�׵ľ��ο�r,������found_filtered��,�����Ƕ�׵Ļ�,��ȡ���������Ǹ����ο����found_filtered��
	for(int i=0; i < found.size(); i++)
	{
		Rect r = found[i];
		int j=0;
		for(; j < found.size(); j++)
			if(j != i && (r & found[j]) == r)
				break;
		if( j == found.size())
			found_filtered.push_back(r);
	}

	//�����ο���Ϊhog�����ľ��ο��ʵ�������Ҫ��΢��Щ,����������Ҫ��һЩ����
	for(int i=0; i<found_filtered.size(); i++)
	{
		Rect r = found_filtered[i];
		r.x += cvRound(r.width*0.1);
		r.width = cvRound(r.width*0.8);
		r.y += cvRound(r.height*0.07);
		r.height = cvRound(r.height*0.8);
		rectangle(src, r.tl(), r.br(), Scalar(0,255,0), 3);
	}

	imwrite("ImgProcessed.jpg",src);
	namedWindow("src",0);
	imshow("src",src);
	waitKey();//ע�⣺imshow֮������waitKey�������޷���ʾͼ��
	

	/******************���뵥��64*128�Ĳ���ͼ������HOG�����ӽ��з���*********************/
	////��ȡ����ͼƬ(64*128��С)����������HOG������
	//Mat testImg = imread("person014142.jpg");
	//Mat testImg = imread("noperson000026.jpg");
	//vector<float> descriptor;
	//hog.compute(testImg,descriptor,Size(8,8));//����HOG�����ӣ���ⴰ���ƶ�����(8,8)
	//Mat testFeatureMat = Mat::zeros(1,3780,CV_32SC1);//����������������������
	////������õ�HOG�����Ӹ��Ƶ�testFeatureMat������
	//for(int i=0; i<descriptor.size(); i++)
	//	testFeatureMat.at<float>(0,i) = descriptor[i];

	////��ѵ���õ�SVM�������Բ���ͼƬ�������������з���
	//int result = svm.predict(testFeatureMat);//�������
	//cout<<"��������"<<result<<endl;

	

	system("pause");
}