/*************************************************************************
** DependencyGraph.hpp                                                  **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2017 Martin Gieseking <martin.gieseking@uos.de>   **
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
		using Dependees = typename std::vector<GraphNode*>;

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

	using NodeMap = std::map<T, GraphNode*>;

	public:
		~DependencyGraph() {
			for (auto &nmpair : _nodeMap)
				delete nmpair.second;
		}

		void insert (const T &key) {
			if (!contains(key))
				_nodeMap[key] = new GraphNode(key);
		}

		void insert (const T &depKey, const T &key) {
			if (contains(key))
				return;
			auto it = _nodeMap.find(depKey);
			if (it != _nodeMap.end()) {
				GraphNode *node = new GraphNode(key);
				it->second->addDependee(node);
				_nodeMap[key] = node;
			}
		}

		void removeDependencyPath (const T &key) {
			auto it = _nodeMap.find(key);
			if (it != _nodeMap.end()) {
				GraphNode *startNode = it->second;
				for (GraphNode *node=startNode; node; node=node->dependent)
					_nodeMap.erase(node->key);
				startNode->deleteDependentsAndSelf();
			}
		}

		std::vector<T> getKeys () const {
			std::vector<T> keys;
			for (auto &entry : _nodeMap)
				keys.push_back(entry.first);
			return keys;
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
			for (auto it=_nodeMap.begin(); it != _nodeMap.end(); ++it) {
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
