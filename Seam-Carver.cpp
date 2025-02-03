#include<opencv2/opencv.hpp>
#include<iostream>
#include<stdexcept>
#include<cstdlib>

using namespace std;
using namespace cv;

string getFileName(string path) {
    int pos = path.find_last_of("/\\");
    return (pos == string::npos)?path:path.substr(pos + 1);
}

void removeHorizontalPath(Mat& img,int* path){
    int h=img.rows;
    int w=img.cols;
    for(int x=0;x<w;x++){
        for(int y=path[x];y<h-1;y++){
            img.at<Vec3b>(y,x)=img.at<Vec3b>(y+1,x);
        }
    }
    img=img(Rect(0,0,w,h-1));
}

void findVertPath(const Mat& energy,int* path){
    int height=energy.rows;
    int width=energy.cols;
    int** cost=new int*[height];
    for(int i=0;i<height;i++){
        cost[i]=new int[width];
    }
    for(int j=0;j<width;j++){
        cost[0][j]=energy.at<uchar>(0,j);
    }
    for(int i=1;i<height;i++){
        for(int j=0;j<width;j++){
            int tl=INT_MAX;
            int t=INT_MAX;
            int tr=INT_MAX;
            
            if(j>0){
                tl=cost[i-1][j-1];
            }
            t=cost[i-1][j];
            if(j<width-1){
                tr=cost[i-1][j+1];
            }

            cost[i][j]=energy.at<uchar>(i,j)+min({tl,t,tr});
        }
    }
    int minIdx=0;
    int minC=cost[height-1][0];
    for(int j=1;j<width;j++){
        if(cost[height-1][j]<minC){
            minC=cost[height-1][j];
            minIdx=j;
        }
    }
    path[height-1]=minIdx;
    for(int i=height-2;i>=0;i--){
        int j=path[i+1];
        int l=INT_MAX;
        int m=cost[i][j];
        int r=INT_MAX;
        
        if(j>0){
            l=cost[i][j-1];
        }
        if(j<width-1){
            r=cost[i][j+1];
        }
        if(l<=m&&l<=r&&j>0){
            path[i]=j-1;
        }
        else if(m<=l&&m<=r){
            path[i]=j;
        }
        else{
            path[i]=j+1;
        }
    }
    for(int i=0;i<height;i++){
        delete[] cost[i];
    }
    delete[] cost;
}

void findHorizontalPath(const Mat& energy,int* path){
    int height=energy.rows;
    int width=energy.cols;
    int** cost=new int*[height];
    for(int i=0;i<height;i++){
        cost[i]=new int[width];
    }
    for(int i=0;i<height;i++){
        cost[i][0]=energy.at<uchar>(i,0);
    }
    for(int j=1;j<width;j++){
        for(int i=0;i<height;i++){
            int topLeft=INT_MAX;
            int left=INT_MAX;
            int bottomLeft=INT_MAX;
            if(i>0){
                topLeft=cost[i-1][j-1];
            }
            left=cost[i][j-1];
            if(i<height-1){
                bottomLeft=cost[i+1][j-1];
            }
            cost[i][j]=energy.at<uchar>(i,j)+min(topLeft,min(left,bottomLeft));
        }
    }
    int minIndex=0;
    int minCost=cost[0][width-1];
    for(int i=1;i<height;i++){
        if(cost[i][width-1]<minCost){
            minCost=cost[i][width-1];
            minIndex=i;
        }
    }
    path[width-1]=minIndex;
    for(int j=width-2;j>=0;j--){
        int i=path[j+1];
        int up=INT_MAX;
        int middle=cost[i][j];
        int down=INT_MAX;
        
        if(i>0){
            up=cost[i-1][j];
        }
        if(i<height-1){
            down=cost[i+1][j];
        }
        if(up<=middle&&up<=down&&i>0){
            path[j]=i-1;
        }
        else if(middle<=up&&middle<=down){
            path[j]=i;
        }
        else{
            path[j]=i+1;
        }
    }
    for(int i=0;i<height;i++){
        delete[] cost[i];
    }
    delete[] cost;
}

void calcEnergyMap(Mat& input,Mat& output){
    Mat gray,gx,gy;
    cvtColor(input,gray,COLOR_BGR2GRAY);
    Sobel(gray,gx,CV_32F,1,0);
    Sobel(gray,gy,CV_32F,0,1);
    magnitude(gx,gy,output);
}

void removeVerticalPath(Mat& img,int* path){
    int height=img.rows;
    int width=img.cols;
    for(int y=0;y<height;y++){
        for(int x=path[y];x<width-1;x++){
            img.at<Vec3b>(y,x)=img.at<Vec3b>(y,x+1);
        }
    }
    img=img(Rect(0,0,width-1,height));
}

int main(int argc,char* argv[]){
    if(argc!=4){
        cout<<"Invalid Number of Arguments Passed..."<<endl;
        return -1;
    }
    string imgPath=string(argv[1]);
    int targetW,targetH;
    try{
        targetW=stoi(argv[2]);
        targetH=stoi(argv[3]);
    }
    catch(exception& e){
        cout<<"The Target Dimensions should be valid numbers..."<<endl;
        exit(-1);
    }

    Mat img=imread(imgPath,IMREAD_COLOR);
    if(img.empty()){
        cout<<"Can't find file..."<<endl;
        return -1;
    }
    int currW=img.cols;
    int currH=img.rows;
    if(targetW<=0||targetH<=0||targetW>currW||targetH>currH){
        cout<<"Invalid target dimensions..."<<endl;
        cout<<"Current Dimensions: ("<<currW<<", "<<currH<<")"<<endl;
        cout<<"Target Dimensions: ("<<targetW<<", "<<targetH<<")"<<endl;
        return -1;
    }
    Mat energyMap,floatImg;
    namedWindow("Seam Visualization",WINDOW_NORMAL);
    
    while(currW>targetW){
        img.convertTo(floatImg,CV_32F);
        calcEnergyMap(floatImg,energyMap);
        normalize(energyMap,energyMap,0,255,NORM_MINMAX);
        energyMap.convertTo(energyMap,CV_8U);
        int* vpath=new int[img.rows];
        findVertPath(energyMap,vpath);
        Mat tmp=img.clone();
        for(int y=0;y<img.rows;++y){
            if(vpath[y]>=0&&vpath[y]<tmp.cols){
                tmp.at<Vec3b>(y,vpath[y])=Vec3b(255,255,255);
            }
        }
        imshow("Seam Visualization",tmp);
        waitKey(1);
        removeVerticalPath(img,vpath);
        currW--;
        delete[] vpath;
    }
    
    while(currH>targetH){
        img.convertTo(floatImg,CV_32F);
        calcEnergyMap(floatImg,energyMap);
        normalize(energyMap,energyMap,0,255,NORM_MINMAX);
        energyMap.convertTo(energyMap,CV_8U);
        int* hpath=new int[img.cols];
        findHorizontalPath(energyMap,hpath);
        Mat tmp=img.clone();
        for(int x=0;x<img.cols;++x){
            if(hpath[x]>=0&&hpath[x]<tmp.rows){
                tmp.at<Vec3b>(hpath[x],x)=Vec3b(255,255,255);
            }
        }
        imshow("Seam Visualization",tmp);
        waitKey(1);
        removeHorizontalPath(img,hpath);
        currH--;
        delete[] hpath;
    }
    
    imshow("Seam Visualization",img);
    string inFileName = getFileName(imgPath);
    string outFileName = "output_" + to_string(currW) + "x" + to_string(currH) + "_"+inFileName ;
    bool success = imwrite(outFileName, img);
    if(success)
        cout<<"Output image saved as: "<< outFileName << endl;
    else
        cout<<"Failed to save output image." << endl;
    waitKey(0);
    return 0;
}