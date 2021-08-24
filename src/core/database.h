#ifndef DATABASE_H
#define DATABASE_H
#include <vector>
#include <memory>
#include <QObject>
#include <QString>
#include <QLineEdit>
#include <QRadioButton>
#include <QComboBox>
#include <QCheckBox>
#include <QTabWidget>
#include <QTableWidgetItem>
#include <Singleton.h>

#include <QDir>
#include <QFile>

#include "mcl.h"
#include "types.h"

#define DATAB (Singleton<DataBase>::GetSingletonPtr())

class DataBase;
class DataNode{
public:
    friend DataBase;
    QString getV(){ return value; }
    QString getK(){ return key; }

	typedef std::vector<std::shared_ptr<DataNode>>::iterator iterator;

	iterator begin() {
		return children.begin();
	}

	iterator end() {
		return children.end();
	}

    int getChildSize(){return children.size();}

    // 访问方式：root->fd("key1")->fd("key5")->getK()
    std::shared_ptr<DataNode> fd( QString key_ ){
        //遍历所有子节点，找到key_节点，返回
        //如果没有找到，则返回null
        for( int i=0; i<children.size();i++ ){
            if( key_ == children[i]->getK() ){
                return children[i];
            }
        }

        return nullptr;
    }

	std::shared_ptr<DataNode> setChild(QString key_, QString val_) {
		auto child = fd(key_);
		if (child) {
			child->setV(val_);
		}
		else {
			fd(key_, val_);
		}
		return fd(key_);
	}

    void setDefaultValue(QString str){
        defaultv = str;
    }

    void reset(){
        setV(defaultv,false);
        for(const auto& child:children){
            child->reset();
        }
    }

    // 访问方式：root->fd("key1")->fd("key5")->getK()
    std::shared_ptr<DataNode> read( QString key_, QString value_ ){
        //读取xml时使用函数
        //遍历所有子节点，找到key_节点，重新赋值为value，并返回
        //如果没有找到，则使用value作为默认值创建，并返回
        for( int i=0; i<children.size();i++ ){
            if( key_ == children[i]->getK()){
                children[i]->setV(value_);
                return children[i];
            }
        }

        std::shared_ptr<DataNode> newDataNode(new DataNode);

        newDataNode->setK(key_);
        newDataNode->setV(value_);
        children.push_back(newDataNode);

        return newDataNode;
    }

    // 访问方式：root->fd("key1","1")->fd("key5","2")->getK()
    std::shared_ptr<DataNode> fd( QString key_, QString value_ ){
        //遍历所有子节点，找到key_节点，返回
        //如果没有找到，则使用value作为默认值创建，并返回
        for( int i=0; i<children.size();i++ ){
            if( key_ == children[i]->getK() ){
                children[i]->setDefaultValue(value_);
                return children[i];
            }      //遍历所有子节点，找到key_节点，返回
            //如果没有找到，则使用value作为默认值创建，并返回
        }

        std::shared_ptr<DataNode> newDataNode(new DataNode);

        newDataNode->setK(key_);
        newDataNode->setV(value_, true);
        newDataNode->setDefaultValue(value_);
        children.push_back(newDataNode);

        return newDataNode;
    }

    void setV(QString value_,bool byWidget=false){
        value = value_;
        if(!byWidget){
            if(updateFunction)
                updateFunction(value);
        }
//         随便写的，先保证不出错。
    }
    void setK(QString key_){
        this->key = key_;

    }

	void clear() {
		children.clear();
	}

    // 输出该节点所包含的所有的数据，进行测试使用
    void printAll();

    /******* 以下定义一些转换函数 ********/
    bool toBool(bool* ok=nullptr){
        if(value == "1"){
            if(ok) *ok = true;
            return true;
        }
        if(value == "0"){
            if(ok) *ok = true;
            return false;
        }
        assert(0);
        if(ok) *ok = false;
        return false;
    }

    int toInt(bool* ok=nullptr){
        bool tempok = false;
        int ret = value.toInt(&tempok);
        assert(tempok);
        if(ok) *ok = tempok;
        return ret;
    }

    double toDouble(bool* ok=nullptr){
        bool tempok = false;
        double ret = value.toDouble(&tempok);
        assert(tempok);
        if(ok) *ok = tempok;
        return ret;
    }

    std::string toStd(){
        return value.toStdString();
    }

	mcl::Vector3f toVector3f() {
		std::istringstream sin(value.toStdString());
		mcl::Vector3f vec;
		for (int i = 0; i < 3; i++) {
			sin >> vec[i];
		}
		return vec;
	}

private:
    QString key;
    QString value;
    QString defaultv;
    std::vector<std::shared_ptr<DataNode>> children;
    bool isBinded=false;
    std::function<void(QString)> updateFunction = [](QString){};
};

//class DataBase:public DataNode, public Singleton<DataBase>
//{
//    Q_OBJECT
//public:
//    friend Singleton<DataBase>;
//public slots:
//    void bindWidgetToNode(QLineEdit* widget,std::shared_ptr<DataNode> node);
//    void bindWidgetToNode(QRadioButton* widget,QRadioButton* widget2 ,std::shared_ptr<DataNode> node);
//    void bindWidgetToNode(QComboBox* widget ,std::shared_ptr<DataNode> node);
//    void bindWidgetToNode(QCheckBox* widget ,std::shared_ptr<DataNode> node);
//    void bindWidgetToNode(QTableWidget *fatherWidget ,QTableWidgetItem *widget ,std::shared_ptr<DataNode> node);
//
//private:
//    DataBase();
//};

#endif // DATABASE_H
