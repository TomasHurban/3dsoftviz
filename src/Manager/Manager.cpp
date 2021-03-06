/*!
 * Manager.cpp
 * Projekt 3DVisual
 */

#include "Manager/Manager.h"
#include "Model/GraphDAO.h"
#include "Util/ApplicationConfig.h"

#include "Importer/ImporterContext.h"
#include "Importer/ImporterFactory.h"
#include "Importer/StreamImporter.h"

#include "Manager/ImportInfoHandlerImpl.h"

#include <memory>

Manager::GraphManager * Manager::GraphManager::manager;

Manager::GraphManager::GraphManager()
{
    manager = this;

    this->activeGraph = NULL;
    this->db = new Model::DB();
    bool error;
    this->graphs = Model::GraphDAO::getGraphs(db->tmpGetConn(), &error);
    
    //runTestCase(1);
}

Manager::GraphManager::~GraphManager()
{
    delete this->db;
    this->db = NULL;
}

Data::Graph* Manager::GraphManager::loadGraph(QString filepath)
{
	bool ok = true;

    AppCore::Core::getInstance()->thr->pause();
    AppCore::Core::getInstance()->messageWindows->showProgressBar();

    // TODO: [ML] better extension getting

	// get extension
	QString extension;
	int dotIndex = -1;

	if (ok) {
		dotIndex = filepath.lastIndexOf (QChar ('.'));
		ok = (dotIndex > 0);
	}


	if (ok) {
		extension = filepath.mid (dotIndex + 1);
	}

	// TODO: [ML] error messages (if no suitable importer has been found)

	// get importer
	std::auto_ptr<Importer::StreamImporter> importer (NULL);
	if (ok) {
		bool importerFound;

		ok =
			Importer::ImporterFactory::createByFileExtension (
				importer,
				importerFound,
				extension.toStdString ()
			)
			&&
			importerFound
		;
	}

    // create stream
    std::string filepathStr = filepath.toStdString ();
    fstream file (filepathStr.c_str (), ios::in | ios::binary);

    // create graph
    std::auto_ptr<Data::Graph> newGraph (NULL);
    if (ok) {
		newGraph.reset (this->createGraph (QString ("new")));
		ok = (newGraph.get () != NULL);
    }

    // create info handler
    std::auto_ptr<Importer::ImportInfoHandler> infoHandler (NULL);
    if (ok) {
    	infoHandler.reset (new ImportInfoHandlerImpl);
    }

    // create context
    std::auto_ptr<Importer::ImporterContext> context (NULL);
    if (ok) {
    	context.reset (
    		new Importer::ImporterContext (
				file,
				*newGraph,
				*infoHandler
			)
    	);
    }

    // perform import
    if (ok) {
    	ok = importer->import (*context);
    }

    // set as active graph
    if (ok) {
    	// ak uz nejaky graf mame, tak ho najprv sejvneme a zavrieme
		if(this->activeGraph != NULL){
			this->saveGraph(this->activeGraph);
			this->closeGraph(this->activeGraph);
		}
		this->activeGraph = newGraph.release ();
    }

    if (ok) {
    	// robime zakladnu proceduru pre restartovanie layoutu
    	AppCore::Core::getInstance()->restartLayout();
    }

	AppCore::Core::getInstance()->messageWindows->closeProgressBar();

    return (ok ? this->activeGraph : NULL);
}

void Manager::GraphManager::saveGraph(Data::Graph* graph)
{
    graph->saveGraphToDB();
}

void Manager::GraphManager::exportGraph(Data::Graph* graph, QString filepath)
{
    // TODO export do GraphML
}

Data::Graph* Manager::GraphManager::createGraph(QString graphname)
{
    Data::Graph* g;
    if(!this->db->tmpGetConn()->isOpen()){
        g = this->emptyGraph();
    } else {
        g = Model::GraphDAO::addGraph(graphname, this->db->tmpGetConn());
    }

    this->graphs.insert(g->getId(), g);
    return g;
}

void Manager::GraphManager::removeGraph(Data::Graph* graph)
{
    this->closeGraph(graph);
    // odstranime graf z DB
    Model::GraphDAO::removeGraph(graph, db->tmpGetConn());
}

void Manager::GraphManager::closeGraph(Data::Graph* graph)
{
	// odstranime graf z working grafov
	this->graphs.remove(graph->getId());
	// TODO zatial pracujeme len s aktivnym grafom, takze deletujeme len jeho, inak treba deletnut dany graf

	this->activeGraph = NULL;
}

Data::Graph* Manager::GraphManager::emptyGraph()
{
	Data::Graph *newGraph = new Data::Graph(1, "simple", 0, 0, NULL);

	return newGraph;
}

Data::Graph* Manager::GraphManager::simpleGraph()
{
	Data::Graph *newGraph = new Data::Graph(1, "simple", 0, 0, NULL);
	Data::Type *type = newGraph->addType("default");
	Data::Type *type2 = newGraph->addType("default2");

	Data::Node *u1 = newGraph->addNode("u1", type);
	Data::Node *u2 = newGraph->addNode("u2", type);
	Data::Node *u3 = newGraph->addNode("u3", type);

	newGraph->addEdge("e1", u1, u2, type2, false);
	newGraph->addEdge("e2", u1, u3, type2, false);
	newGraph->addEdge("e3", u2, u3, type2, false);

	return newGraph;
}

Manager::GraphManager* Manager::GraphManager::getInstance()
{
	if(manager == NULL)
	{

            manager = new Manager::GraphManager();

	}

	return manager;
}

void Manager::GraphManager::runTestCase( qint32 action )
{
	switch(action) {
		case 1:
		case 2: {
			//inicializacia
			qDebug() << "TestCase initialization";
			bool error;
			Data::Graph* g = Model::GraphDAO::addGraph("testCase1",this->db->tmpGetConn()); //vytvorenie grafu
			g->addLayout("testCase1 layout 1"); //pridanie layoutu
			Data::GraphLayout* gl2 = g->addLayout("testCase1 layout 2"); //pridanie layoutu
			g->addLayout("testCase1 layout 3"); //pridanie layoutu
			qDebug() << "layouty grafu: " << g->toString();
			QMap<qlonglong,Data::GraphLayout*> layouts = g->getLayouts(&error);
			foreach(qlonglong i, layouts.keys()) { //vypis layoutov
				qDebug() << layouts.value(i)->toString();
			}

			g->selectLayout(gl2); //vybratie layoutu
			Data::Type* t1 = g->addType("type1"); //pridanie typu
			Data::Type* t2 = g->addType("type2"); //pridanie typu
			Data::Type* t3 = g->addType("type3"); //pridanie typu
			Data::Type* t4 = g->addType("type4"); //pridanie typu
			Data::Type* t5 = g->addType("type5"); //pridanie typu
			Data::Type* t6 = g->addType("type6"); //pridanie typu
			Data::MetaType* mt1 = g->addMetaType("metaType1"); //pridanie metatypu
			Data::MetaType* mt2 = g->addMetaType("metaType2"); //pridanie metatypu
			Data::MetaType* mt3 = g->addMetaType("metaType3"); //pridanie metatypu

			for(qlonglong i=0;i<100;i++) {
				if(i%3==1) {
					g->addNode("node",t2);
				} else if(i%3==2) {
					g->addNode("node",t3);
				} else {
					g->addNode("node",t1);
				}
			}

			QMap<qlonglong, osg::ref_ptr<Data::Node> >* gNodes = g->getNodes();
			osg::ref_ptr<Data::Node> n1;
			osg::ref_ptr<Data::Node> n2;
			qlonglong iteration = 0;
			foreach(qlonglong i, gNodes->keys()) {
				if(iteration==0) {
					n1 = gNodes->value(i);
				} else {
					n2 = gNodes->value(i);
					if(iteration%3==1) {
						g->addEdge("edge",n1,n2,t5,true);
					} else if(iteration%3==2) {
						g->addEdge("edge",n1,n2,t6,true);
					} else {
						g->addEdge("edge",n1,n2,t4,true);
					}
					n1 = gNodes->value(i);
				}
				iteration++;
			}
			n1 = NULL;
			n2 = NULL;

			qDebug() << "Nodes count: " << g->getNodes()->size();
			qDebug() << "Types count: " << g->getTypes()->size();
			qDebug() << "Edges count: " << g->getEdges()->size();

			switch(action) {
				case 1: //testovanie remove metod
					qDebug() << "Starting testCase 1";

					qDebug() << "Removing type t1";
					g->removeType(t1);
					qDebug() << "type t1 removed";

					qDebug() << "Counts after the type t1 was removed";
					qDebug() << "Nodes count: " << g->getNodes()->size();
					qDebug() << "Types count: " << g->getTypes()->size();
					qDebug() << "Edges count: " << g->getEdges()->size();

					qDebug() << "Ending testCase 1";
					break;
				case 2:
					qDebug() << "Starting testCase 2";

					osg::ref_ptr<Data::Node> selectedNode = g->getNodes()->values().at(10);

					qDebug() << "Removing node: " << selectedNode->toString();
					g->removeNode(selectedNode);

					qDebug() << "Counts after the type t1 was removed";
					qDebug() << "Nodes count: " << g->getNodes()->size();
					qDebug() << "Types count: " << g->getTypes()->size();
					qDebug() << "Edges count: " << g->getEdges()->size();

					qDebug() << "Ending testCase 2";
					break;
			}

			//cleanup
			if(Model::GraphDAO::removeGraph(g,this->db->tmpGetConn())) {
				qDebug() << "graph successfully removed from db";
			} else {
				qDebug() << "could not be removed from db";
			}

			delete g;
			g = NULL;
			break;
		}
		case 3: {
			Data::Graph* g = Model::GraphDAO::addGraph("testCase1",this->db->tmpGetConn()); //vytvorenie grafu
			Data::Type* t1 = g->addType("type");
			Data::Type* t2 = g->addType("type2");
			osg::ref_ptr<Data::Node> n1 = g->addNode("node1",t1);
			osg::ref_ptr<Data::Node> n2 = g->addNode("node2",t1);
			osg::ref_ptr<Data::Edge> e1 = g->addEdge("edge1",n1,n2,t2,true);
			g->addEdge("edge2",n1,n2,t2,true);
			g->addEdge("edge3",n1,n2,t2,true);
			g->addEdge("edge4",n1,n2,t2,true);
			g->removeNode(n1);
			n1 = NULL;
			qDebug() << "node should be deleted";
			e1 = NULL;
			qDebug() << "edge should be deleted";

			delete g;
			g = NULL;
			qDebug() << "graph deleted";
			n2 = NULL;
			break;
		}
		case 4: {
			Data::Graph* g = Model::GraphDAO::addGraph("testCase1",this->db->tmpGetConn()); //vytvorenie grafu
			Data::Type* t1 = g->addType("type");
			Data::Type* t2 = g->addType("type2");
			osg::ref_ptr<Data::Node> n1 = g->addNode("node1",t1);
			osg::ref_ptr<Data::Node> n2 = g->addNode("node2",t1);
			osg::ref_ptr<Data::Edge> e1 = g->addEdge("edge1",n1,n2,t2,true);
			g->removeEdge(e1);
			e1 = NULL;
			qDebug() << "edge should be deleted";
			n2 = NULL;
			n1 = NULL;

			delete g;
			g = NULL;
			qDebug() << "graph deleted";
			break;
		}

		case 5: {
			Data::Graph* g = Model::GraphDAO::addGraph("testCase1",this->db->tmpGetConn()); //vytvorenie grafu
			g->selectLayout(g->addLayout("layout"));
			Data::Type* t1 = g->addType("type");
			Data::MetaType* t2 = g->addMetaType("type2");
			Data::Type* t3 = g->addType("type3");
			osg::ref_ptr<Data::Node> n1 = g->addNode("node1",t1);
			osg::ref_ptr<Data::Node> n2 = g->addNode("node2",t1);
			osg::ref_ptr<Data::Edge> e1 = g->addEdge("edge1",n1,n2,t2,true);
			g->addEdge("edge2",n1,n2,t2,true);
			g->addEdge("edge3",n1,n2,t3,true);
			g->addEdge("edge4",n1,n2,t3,true);
			g->removeNode(n1);
			n1 = NULL;
			qDebug() << "node should be deleted";
			e1 = NULL;
			qDebug() << "edge should be deleted";
			n2 = NULL;

			delete g;
			g = NULL;
			qDebug() << "graph deleted";
			break;
		}

		case 6: {
			Data::Graph* g = Model::GraphDAO::addGraph("testCase1",this->db->tmpGetConn()); //vytvorenie grafu
			g->selectLayout(g->addLayout("layout"));
			Data::Type* t1 = g->addType("type");
			Data::MetaType* t2 = g->addMetaType("type2");
			osg::ref_ptr<Data::Edge> e1 = g->addEdge("edge1",g->addNode("node1",t1),g->addNode("node2",t1),t2,true);
			g->removeEdge(e1);
			e1 = NULL;
			qDebug() << "edge should be deleted";

			delete g;
			g = NULL;
			qDebug() << "graph deleted";
			break;
		}

		case 7: {
			Data::Graph* g = Model::GraphDAO::addGraph("testCase1",this->db->tmpGetConn()); //vytvorenie grafu
			g->selectLayout(g->addLayout("layout"));
			Data::Type* t1 = g->addType("type");
			g->addEdge("edge1",g->addNode("node1",t1),g->addNode("node2",t1),g->addMetaType("type2"),true);
			g->removeType(t1);

			delete t1;
			t1 = NULL;

			qDebug() << "type should be deleted";

			delete g;
			g = NULL;
			qDebug() << "graph deleted";
			break;
		}
	}
}
