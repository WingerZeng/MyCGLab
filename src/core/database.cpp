#include "database.h"

std::shared_ptr<DataNode> DataNode::clone() const
{
	std::shared_ptr<DataNode> newnode = std::make_shared<DataNode>();
	newnode->setK(key);
	newnode->setV(value);
	newnode->setDefaultValue(defaultv);
	for (const auto& child : children) {
		newnode->children.push_back(child->clone());
	}
	return newnode;
}

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
