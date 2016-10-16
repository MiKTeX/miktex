/*************************************************************************
** DependencyGraph.hpp                                                  **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2016 Martin Gieseking <martin.gieseking@uos.de>   **
**                                                                      **
** This program is free software; you can redistribute it and/or        **
** modify it under the terms of the GNU General Public License as       **
** published by the Free Software Foundation; either version 3 of       **
** the License, or (at your option) any later version.                  **
**                                                                      **
** This program is distributed in the hope that it will be useful, but  **
** WITHOUT ANY WARRANTY; without even the implied warranty of           **
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the         **
** GNU General Public License for more details.                         **
**                                                                      **
** You should have received a copy of the GNU General Public License    **
** along with this program; if not, see <http://www.gnu.org/licenses/>. **
*************************************************************************/

#ifndef DEPENDENCYGRAPH_HPP
#define DEPENDENCYGRAPH_HPP

#include <map>
#include <vector>

template <typename T>
class DependencyGraph
{
	struct GraphNode {
		typedef typename std::vector<GraphNode*> Dependees;

		GraphNode (const T &k) : key(k), dependent(0) {}

		void addDependee (GraphNode *node) {
			if (node) {
				node->dependent = this;
				dependees.push_back(node);
			}
		}

		void deleteDependentsAndSelf () {
			if (dependent)
				dependent->deleteDependentsAndSelf();
			for (typename Dependees::iterator it = dependees.begin(); it != dependees.end(); ++it)
				(*it)->dependent = 0;
			delete this;
		}

		T key;
		GraphNode *dependent;
		Dependees dependees;
	};

	typedef std::map<T, GraphNode*> NodeMap;

	public:
		~DependencyGraph() {
			for (typename NodeMap::iterator it=_nodeMap.begin(); it != _nodeMap.end(); ++it)
				delete it->second;
		}

		void insert (const T &key) {
			if (!contains(key))
				_nodeMap[key] = new GraphNode(key);
		}

		void insert (const T &depKey, const T &key) {
			if (contains(key))
				return;
			typename NodeMap::iterator it = _nodeMap.find(depKey);
			if (it != _nodeMap.end()) {
				GraphNode *node = new GraphNode(key);
				it->second->addDependee(node);
				_nodeMap[key] = node;
			}
		}

		void removeDependencyPath (const T &key) {
			typename NodeMap::iterator it = _nodeMap.find(key);
			if (it != _nodeMap.end()) {
				GraphNode *startNode = it->second;
				for (GraphNode *node=startNode; node; node=node->dependent)
					_nodeMap.erase(node->key);
				startNode->deleteDependentsAndSelf();
			}
		}

		void getKeys (std::vector<T> &keys) const {
			for (typename NodeMap::const_iterator it=_nodeMap.begin(); it != _nodeMap.end(); ++it)
				keys.push_back(it->first);
		}

		bool contains (const T &value) const {
			return _nodeMap.find(value) != _nodeMap.end();
		}

		bool empty () const {
			return _nodeMap.empty();
		}

#if 0
		void writeDOT (std::ostream &os) const {
			os << "digraph {\n";
			for (typename NodeMap::const_iterator it=_nodeMap.begin(); it != _nodeMap.end(); ++it) {
				GraphNode *node = it->second;
				if (node->dependent)
					os << (node->key) << " -> " << (node->dependent->key) << ";\n";
				else if (node->dependees.empty())
					os << (node->key) << ";\n";
			}
			os << "}\n";
		}
#endif

	private:
		NodeMap _nodeMap;
};

#endif
