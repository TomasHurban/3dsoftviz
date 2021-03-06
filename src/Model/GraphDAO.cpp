/*!
 * GraphDAO.cpp
 * Projekt 3DVisual
 */
#include "Model/GraphDAO.h"

Model::GraphDAO::GraphDAO(void)
{
}

Model::GraphDAO::~GraphDAO(void)
{
}

QMap<qlonglong, Data::Graph*> Model::GraphDAO::getGraphs(QSqlDatabase* conn, bool* error)
{
    QMap<qlonglong, Data::Graph*> qgraphs;
    *error = FALSE;

    if(conn==NULL || !conn->isOpen()) { //check if we have connection
        qDebug() << "[Model::GraphDAO::getGraphs] Connection to DB not opened.";
        *error = TRUE;
        return qgraphs;
    }
    
    //get all graphs with their max element id
    QSqlQuery* query = new QSqlQuery(*conn);
    query->prepare("SELECT g.graph_id, g.graph_name, (CASE WHEN MAX(l.layout_id) IS NOT NULL THEN MAX(l.layout_id) ELSE 0 END) AS layout_id, (CASE WHEN MAX(ele_id) IS NOT NULL THEN MAX(ele_id) ELSE 0 END) AS ele_id FROM ("
        "("
        "SELECT MAX(node_id) AS ele_id, graph_id FROM nodes "
        "GROUP BY graph_id"
        ") UNION ALL ("
        "SELECT MAX(edge_id) AS ele_id, graph_id FROM edges "
        "GROUP BY graph_id"
        ")"
        ") AS foo "
        "RIGHT JOIN graphs AS g ON "
        "g.graph_id = foo.graph_id "
        "LEFT JOIN layouts AS l ON "
        "l.graph_id = g.graph_id "
        "GROUP BY g.graph_id, g.graph_name");

    if(!query->exec()) {
        qDebug() << "[Model::GraphDAO::getGraphs] Could not perform query on DB: " << query->lastError().databaseText();
        *error = TRUE;
        return qgraphs;
    }
    
    while(query->next()) {
        qgraphs.insert(query->value(0).toLongLong(), new Data::Graph(query->value(0).toLongLong(),query->value(1).toString(),query->value(2).toLongLong(),query->value(3).toLongLong(),conn));
    }
    return qgraphs;
}

Data::Graph* Model::GraphDAO::addGraph(QString graph_name, QSqlDatabase* conn)
{
    if(conn==NULL || !conn->isOpen()) { //check if we have connection
        qDebug() << "[Model::GraphDAO::addGraph] Connection to DB not opened.";
        return NULL;
    }

    QSqlQuery* query = new QSqlQuery(*conn);
    query->prepare("INSERT INTO graphs (graph_name) VALUES (:graph_name) RETURNING graph_id");
    query->bindValue(":graph_name",graph_name);
    if(!query->exec()) {
        qDebug() << "[Model::GraphDAO::addGraph] Could not perform query on DB: " << query->lastError().databaseText();
        return NULL;
    }

    if(query->next()) {
        Data::Graph* graph = new Data::Graph(query->value(0).toLongLong(),graph_name,0,0,conn);
        graph->setIsInDB();
        qDebug() << "[Model::GraphDAO::addGraph] Graph was added to DB: " << graph->toString();
        return graph;
    } else {
        qDebug() << "[Model::GraphDAO::addGraph] Graph was not added to DB: " << query->lastQuery();
        return NULL;
    }
}

bool Model::GraphDAO::addGraph( Data::Graph* graph, QSqlDatabase* conn )
{
    if(conn==NULL || !conn->isOpen()) { //check if we have connection
        qDebug() << "[Model::GraphDAO::addGraph] Connection to DB not opened.";
        return false;
    }
    
    if(graph==NULL) {
        qDebug() << "[Model::GraphDAO::addGraph] Invalid parameter - graph is NULL";
        return false;
    }
    
    if(graph->isInDB()) return true; //graph already in DB
    
    QSqlQuery* query = new QSqlQuery(*conn);
    query->prepare("INSERT INTO graphs (graph_name) VALUES (:graph_name) RETURNING graph_id");
    query->bindValue(":graph_name",graph->getName());
    if(!query->exec()) {
        qDebug() << "[Model::GraphDAO::addGraph] Could not perform query on DB: " << query->lastError().databaseText();
        return NULL;
    }

    if(query->next()) {
        graph->setId(query->value(0).toLongLong());
        graph->setIsInDB();
        qDebug() << "[Model::GraphDAO::addGraph] Graph was added to DB and it's ID was set to: " << graph->getId();
        return true;
    } else {
        qDebug() << "[Model::GraphDAO::addGraph] Graph was not added to DB: " << query->lastQuery();
        return false;
    }
}


bool Model::GraphDAO::removeGraph(Data::Graph* graph, QSqlDatabase* conn)
{
    if(conn==NULL || !conn->isOpen()) { //check if we have connection
        qDebug() << "[Model::GraphDAO::removeGraph] Connection to DB not opened.";
        return false;
    }
    
    if(graph==NULL) {
        qDebug() << "[Model::GraphDAO::removeGraph] Invalid parameter - graph is NULL";
        return false;
    }

    if(!graph->isInDB()) return true;

    QSqlQuery* query = new QSqlQuery(*conn);
    query->prepare("DELETE FROM graphs WHERE graph_id = :graph_id");
    query->bindValue(":graph_id",graph->getId());
    if(!query->exec()) {
        qDebug() << "[Model::GraphDAO::removeGraph] Could not perform query on DB: " << query->lastError().databaseText();
        return false;
    }

	//TODO: mala by byt moznost nastavit isInDB priznak grafu

    return true;
}

QString Model::GraphDAO::setName(QString name,Data::Graph* graph, QSqlDatabase* conn)
{
    //bolo by lepsie prerobit na vracanie error flagu aby sa vedelo ci problem nastal kvoli nedostatocnym parametrom alebo kvoli chybe insertu
    if(conn==NULL || !conn->isOpen()) { //check if we have connection
        qDebug() << "[Model::GraphDAO::setName] Connection to DB not opened.";
        return NULL;
    }
    if(graph==NULL) {
        qDebug() << "[Model::GraphDAO::setName] Invalid parameter - graph is NULL";
        return NULL;
    }
    
    if(!graph->isInDB()) {
        if(!Model::GraphDAO::addGraph(graph, conn)) { //could not insert graph into DB
            qDebug() << "[Model::GraphDAO::setName] Could not update graph name in DB. Graph is not in DB.";
            return NULL;
        }
    }

    QSqlQuery* query = new QSqlQuery(*conn);
    query->prepare("UPDATE graphs SET graph_name = :graph_name WHERE graph_id = :graph_id");
    query->bindValue(":graph_id",graph->getId());
    query->bindValue(":graph_name",name);
    if(!query->exec()) {
        qDebug() << "[Model::GraphDAO::setName] Could not perform query on DB: " << query->lastError().databaseText();
        return NULL;
    }

    return name;
}

QMap<QString,QString> Model::GraphDAO::getSettings( Data::Graph* graph, QSqlDatabase* conn, bool* error)
{
    QMap<QString,QString> settings;
    *error = FALSE;
    
    if(conn==NULL || !conn->isOpen()) { //check if we have connection
        qDebug() << "[Model::GraphDAO::getSettings] Connection to DB not opened.";
        *error = TRUE;
        return settings;
    }

    if(graph==NULL) {
        qDebug() << "[Model::GraphDAO::getSettings] Invalid parameter - graph is NULL";
        *error = TRUE;
        return settings;
    }

    if(!graph->isInDB()) {
        qDebug() << "[Model::GraphDAO::getSettings] Graph is not in DB";
        *error = TRUE;
        return settings;
    }
    
    QSqlQuery* query = new QSqlQuery(*conn);
    query->prepare("SELECT val_name, val FROM graph_settings WHERE graph_id = :graph_id");
    query->bindValue(":graph_id",graph->getId());
    if(!query->exec()) {
        qDebug() << "[Model::GraphDAO::getSettings] Could not perform query on DB: " << query->lastError().databaseText();
        *error = TRUE;
        return settings;
    }

    while(query->next()) {
        settings.insert(query->value(0).toString(),query->value(1).toString());
    }
    
    return settings;
}

