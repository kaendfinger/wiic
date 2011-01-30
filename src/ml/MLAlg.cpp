#include "MLAlg.h"
    
MLAlg::MLAlg(type t) 
{
	tp=t;
	
	// *** Parameters ***
	// KNN
	K=10; 
	
	// SVM
	svmparams.svm_type = CvSVM::C_SVC;  // C_SVC o NU_SVC
	svmparams.kernel_type = CvSVM::RBF; // SIGMOID or RBF;
	svmparams.gamma = 1.0;  // for poly/rbf/sigmoid
	svmparams.coef0 = 0.0;  // for poly/sigmoid
	svmparams.C = 0.5;
	svmparams.nu = 0.5;
	
	// DT
	dtparams.max_categories = 6;
	
	// Boost
	CvBoostParams params;
	params.boost_type = CvBoost::DISCRETE;
	params.weak_count = 20;
	params.split_criteria = CvBoost::DEFAULT;
	params.weight_trim_rate = 0;
	
	// RT
	rtparams.max_categories = 6;	
}
	
void MLAlg::setType(const string& st) 
{
     tp = NONE;
	for (int i = 0 ; i < NUMTYPES ; i++) {
    	if (st == MLAlgName[i]) tp = (type)i;
	}
}
	
    
void MLAlg::train(const CvMat *trainIn, const CvMat *trainOut)
{
	cout << "Training " << MLAlgName[tp] << " with " << trainIn->rows << " training samples." << endl;
	CvMat* var_type = 0;
	var_type = cvCreateMat( trainIn->cols + 1, 1, CV_8U );
	cvSet( var_type, cvScalarAll(CV_VAR_NUMERICAL) ); 
	CV_MAT_ELEM(*var_type, uchar, trainIn->cols, 0) = CV_VAR_CATEGORICAL;
	
	switch (tp) {
    	case KNN:
		knn.train( trainIn, trainOut, 0, false, K );
		break;
    
		case Bayes:
		bayes.train( trainIn, trainOut );
		break;
    
		case SVM:
		svm.train_auto( trainIn, trainOut, 0, 0, svmparams );
		break;
		
		case DT:
		dt.train(trainIn, CV_ROW_SAMPLE, trainOut, 0, 0, var_type, 0, dtparams);
		break;
		
		case Boost:
		boost.train(trainIn, CV_ROW_SAMPLE, trainOut, 0, 0, var_type, 0, boostparams);
		break;
		
		case RT:
		rt.train(trainIn, CV_ROW_SAMPLE, trainOut, 0, 0, var_type, 0, rtparams);
		break;
				
    	default:
		cout << "ML method not implemented yet." << endl;	
	}
}

void MLAlg::validate(const CvMat *validateIn, const CvMat *validateOut) {
	cout << "Validating "  << MLAlgName[tp] << " with " << validateIn->rows << " training samples." << endl;

	CvMat sample; // = cvCreateMat(1, mldata.getNumFeatures(), CV_32FC1 );
	float cat; int err=0;
	CvDTreeNode* n = 0;
	
	for (int v=0; v<validateIn->rows; v++) {
    	cvGetRow(validateIn, &sample, v);
    	cat = validateOut->data.fl[v]; float r=cat+1;
    	switch (tp) {
			case KNN:
	    	r = knn.find_nearest(&sample,K); 
			break;
			
			case Bayes:
	    	r = bayes.predict(&sample); 
			break;
			
			case SVM:
	    	r = svm.predict(&sample); 
			break;
			
			case DT:
			n = dt.predict(&sample);
			if(n)
				r = (float)n->value;
			break;
			
			case Boost:
			r = boost.predict(&sample); 
			break;
			
			case RT:
			r = rt.predict(&sample);
			break;
			
    		default:
			cout << "ML method not implemented yet." << endl;
    	}
    	err += (cat!=r);
    	cout << "True value: " << cat << " - KNN: " << r << "  " << (r==cat?"":"ERROR") << endl;
	}
	cout << "Errors = " << err << "/" << validateIn->rows << " - Accuracy = " << (1.0f-(float)err/validateIn->rows)*100 << " %" << endl;
	cout << "Done." << endl;
}