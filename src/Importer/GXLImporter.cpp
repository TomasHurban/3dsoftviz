#include "Importer/GXLImporter.h"
//-----------------------------------------------------------------------------
#include "Importer/GraphOperations.h"
#include "Importer/ReadNodesStore.h"
//-----------------------------------------------------------------------------
#include "Util/Adapter_iostream_to_QIODevice.h"
//-----------------------------------------------------------------------------
#include <QtXml/QXmlStreamReader>
//-----------------------------------------------------------------------------
#include <memory>

namespace Importer {

bool GXLImporter::import (
	ImporterContext &context
) {
	Util::Adapter_iostream_to_QIODevice dev (context.getStream());
	QXmlStreamReader xml (&dev);

	GraphOperations graphOp (context.getGraph ());

	ReadNodesStore readNodes;

	bool ok = true;

	if (ok) {
		ok = !xml.hasError ();

		context.getInfoHandler ().reportError (ok, std::wstring (L"XML format error."));
	}

	bool edgeOrientedDefault = false;
	bool edgeOrientedDefaultForce = false;

	Data::Type *edgeType = NULL;
	Data::Type *nodeType = NULL;
	(void)graphOp.addDefaultTypes (edgeType, nodeType);

	while (ok && !xml.atEnd ()) {
		QXmlStreamReader::TokenType token;
		if (ok) {
			token = xml.readNext();
		}

		if (ok) {
			if (
				(token == QXmlStreamReader::StartElement)
				&&
				(xml.name () == "graph")
			) {
				QXmlStreamAttributes attrs = xml.attributes();

				QString graphName;
				if (ok) {
					graphName = attrs.value ("id").toString ();

					ok = ("" != graphName);

					context.getInfoHandler ().reportError (ok, std::wstring (L"Graph name can not be empty."));
				}

				if (ok) {
					// ok = (graphName == context.getGraph ().setName (graphName));

					context.getInfoHandler ().reportError (ok, std::wstring (L"Unable to set graph name."));
				}

				if (ok) {
					QStringRef graphEdgeMode = attrs.value ("edgemode");
					if (!graphEdgeMode.isEmpty ()) {
						if (graphEdgeMode == "defaultdirected") {
							edgeOrientedDefault = true;
							edgeOrientedDefaultForce = false;
						} else if (graphEdgeMode == "defaultundirected") {
							edgeOrientedDefault = false;
							edgeOrientedDefaultForce = false;
						} else if (graphEdgeMode == "directed") {
							edgeOrientedDefault = true;
							edgeOrientedDefaultForce = true;
						} else if (graphEdgeMode == "undirected") {
							edgeOrientedDefault = false;
							edgeOrientedDefaultForce = true;
						} else {
							ok = false;

							context.getInfoHandler ().reportError (std::wstring (L"Invalid edgemode value."));
						}
					}
				}
			}

			if (
				(token == QXmlStreamReader::StartElement)
				&&
				(xml.name () == "node")
			) {
				QXmlStreamAttributes attrs = xml.attributes();

				QString nodeName;
				if (ok) {
					nodeName = attrs.value ("id").toString ();

					ok = !(nodeName.isEmpty ());

					context.getInfoHandler ().reportError (ok, std::wstring (L"Node ID can not be empty."));
				}

				osg::ref_ptr<Data::Node> node (NULL);
				if (ok) {
					node = context.getGraph ().addNode (nodeName, nodeType);

					ok = node.valid ();

					context.getInfoHandler ().reportError (ok, std::wstring (L"Unable to add new node."));
				}

				if (ok) {
					readNodes.addNode (nodeName, node);
				}
			}

			if (
				(token == QXmlStreamReader::StartElement)
				&&
				(xml.name () == "edge")
			) {
				QXmlStreamAttributes attrs = xml.attributes();

				bool oriented = edgeOrientedDefault;
				if (ok) {
					QStringRef edgeIsDirected = attrs.value ("isdirected");

					if (!edgeIsDirected.isEmpty ()) {
						if (edgeIsDirected == "true") {
							if (
								(!edgeOrientedDefaultForce)
								||
								(edgeOrientedDefault)
							) {
								oriented = true;
							} else {
								ok = false;
								context.getInfoHandler ().reportError (std::wstring (L"Can not replace global edge mode with edgeIsDirected=\"true\"."));
							}
						} else if (edgeIsDirected == "false") {
							if (
								(!edgeOrientedDefaultForce)
								||
								(!edgeOrientedDefault)
							) {
								oriented = false;
							} else {
								ok = false;
								context.getInfoHandler ().reportError (std::wstring (L"Can not replace global edge mode with edgeIsDirected=\"false\"."));
							}
						} else {
							context.getInfoHandler ().reportError (ok, std::wstring (L"Invalid edgeIsDirected value."));
						}
					}
				}

				QString nodeFromName;
				if (ok) {
					nodeFromName = attrs.value ("from").toString ();

					ok = !(nodeFromName.isEmpty ());

					context.getInfoHandler ().reportError (ok, std::wstring (L"Edge \"from\" attribute can not be empty."));
				}

				QString nodeToName;
				if (ok) {
					nodeToName = attrs.value ("to").toString ();

					ok = !(nodeToName.isEmpty ());

					context.getInfoHandler ().reportError (ok, std::wstring (L"Edge \"to\" attribute can not be empty."));
				}

				QString edgeName = nodeFromName + nodeToName;

				if (ok) {
					ok = readNodes.contains (nodeFromName);

					context.getInfoHandler ().reportError (ok, std::wstring (L"Edge references invalid source node."));
				}

				if (ok) {
					ok = readNodes.contains (nodeToName);

					context.getInfoHandler ().reportError (ok, std::wstring (L"Edge references invalid destination node."));
				}

				if (ok) {
					osg::ref_ptr<Data::Edge> edge = context.getGraph().addEdge(
						edgeName,
						readNodes.get (nodeFromName),
						readNodes.get (nodeToName),
						edgeType,
						oriented
					);

					ok = edge.valid ();

					context.getInfoHandler ().reportError (ok, std::wstring (L"Unable to add new edge."));
				}
			}
		}

		if (ok) {
			ok = !xml.hasError ();

			context.getInfoHandler ().reportError (ok, std::wstring (L"XML format error."));
		}
	}

	xml.clear ();

	return ok;
}

} // namespace
