#include "VrApp.h"
#include <fstream>
#include <cstdio>
#include <iostream>
#include <string>
using namespace std;

VrApp::VrApp()
{
    //ctor
}

void VrApp::loadCategorizedModels()
{
    ifstream file;

    for(unsigned int c=0;c<categories.size();c++)
    {
        int num_models=0;
        while(1)
        {
            int num_materials=0;
            string name_model=getFileName(c,num_models,0);
            file.open(name_model+".obj");
            if (file.is_open())
            {
                do{
                    file.close();
                    file.open(name_model+"_"+to_string(num_materials+1)+".mtl");
                    if (file.is_open())
                    {
                        num_materials++;
                    }
                }while(file.is_open());
                categories[c].models.push_back(num_materials);
                num_models++;
            }
            else
            {
                break;
            }
        }

    }

}

std::string VrApp::getFileName(int category, int id, int mat)
{
    std::string fileName;
    char name[50];

    sprintf(name,"Models/Architectural/%ss/%s%d/%s%d",
            categories[category].name.c_str(),
            categories[category].name.c_str(),
            id+1,
            categories[category].name.c_str(),
            id+1);

    fileName=name;

    if (mat)
    {
        fileName+="_"+to_string(mat);
    }
    return fileName;
}
/*
    for(int c=0; c<app.categories.size();c++)
    {
        cout<<app.categories[c].name<<endl;
        cout<<"===================="<<endl;

        for(int m=0; m<app.categories[c].models.size();m++)
        {
            cout<<app.getFileName(c,m,0)<<".obj"<<endl;
            for(int t=1; t<=app.categories[c].models[m];t++)
            {
                cout<<app.getFileName(c,m,t)<<".mtl"<<endl;
            }
        }

    }
//*/
