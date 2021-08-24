#include "database.h"

void DataNode::printAll()
{
    qDebug() << getK() << ":" << getV() << ";";
    if(children.size()!=0){
        qDebug() << "The child node is:";
        for( int i=0; i<children.size(); i++ ){
            children[i]->printAll();
        }
        qDebug() << "The children of" << getK() << ":" << getV() << ";" << "are over";
    }
}
