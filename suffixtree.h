#ifndef SUFFIXTREE_H
#define SUFFIXTREE_H

#include <vector>
#include <queue>
#include <stack>
#include <iostream>
#include <string>

class SuffixTree
{
public:

    class Node
    {
    public:

        int start;

        int *end;

        Node *children[27];

        Node *suffixLink;

        Node(int start = 0, int *end = nullptr) : start(start), end(end), suffixLink(nullptr)
        {
            for (int i = 0; i < 27; i++)
                children[i] = nullptr;
        }

        int edgeLength()
        {
//            if (end == nullptr)
//            {
//                std::cout << "ебать ты даун\n";
//                return 0;
//            } else
            return *end - start + 1;
        }
    };

    SuffixTree(std::string text)
    {
        _root = new Node();

        _text = text + "$";

        _pos = -1;

        // initialize remainder
        _remainder = 0;

        // initialize active point
        _activeNode = _root;
        _activeEdge = -1;
        _activeLength = 0;

        // construct the suffix tree from the given _text

        // Ukkonen's Algorithm
        for (int step = 0; step < _text.size(); step++)
            extend(step); // extend the suffix tree with character _text[step]

        // *** before finishing the program, let's inspect the suffix tree ***
        std::cout << std::endl;
        std::cout << "remainder = " << _remainder << std::endl;
        if (_activeNode == _root)
            std::cout << "active node = root " << std::endl;
        else
            std::cout << "node " << _text.substr(_activeNode->start, _activeNode->edgeLength()) << std::endl;
        std::cout << "active edge = " << _activeEdge << std::endl;
        std::cout << "active length = " << _activeLength << std::endl;
        std::cout << std::endl;

        printNodes(_root);
        std::cout << std::endl;
    }

    void printNodes(Node *currentNode)
    {
        if (currentNode)
        {
            for (int i = 0; i < 27; i++)
            {
                // *** when deleting the suffix tree, let's check out the value of nodes ***
                if (currentNode->children[i] != nullptr)
                {
                    if (currentNode->end != nullptr)
                        std::cout << _text.substr(currentNode->start, currentNode->edgeLength());
                    else
                        std::cout << "root";

                    std::cout << "===>"
                              << _text.substr(currentNode->children[i]->start, currentNode->children[i]->edgeLength());

                    std::cout << std::endl;

                }
                printNodes(currentNode->children[i]);
            }
        }
    }

    ~SuffixTree()
    {
        destruct(_root);
    }

    void destruct(Node *currentNode)
    {
        if (currentNode)
        {
            for (int i = 0; i < 27; i++)
                destruct(currentNode->children[i]);
            delete currentNode;
        }
    }

    bool walkDown(Node *next)
    {
        if (_activeLength >= next->edgeLength())
        {
            _activeNode = next;
            _activeEdge = _text[_activeEdge + next->edgeLength()] - 97;
            _activeLength -= next->edgeLength();
            return true;
        }
        return false;
    }

    void extend(int step)
    {
        _pos = step;

        _remainder++;

        Node *lastSplitNode = nullptr;

        int nextChar = _text[step] != '$' ? _text[step] - 97
                                          : 26; // next character to extend in the suffix tree as an integer, the $ symbol will be the last child.

        while (_remainder > 0)
        {
            if (_activeLength == 0)
                _activeEdge = nextChar; // if the active point has not moved, let us look for child of active node with first character _text[step]

            if (_activeNode->children[_activeEdge] == nullptr) // child does not exist
            {
                _activeNode->children[_activeEdge] = new Node(step, &_pos); // create child with label _text[step]

                if (lastSplitNode != nullptr)
                {
                    lastSplitNode->suffixLink = _activeNode;
                    lastSplitNode = nullptr; // reset lastSplitNode variable
                }

            } else
            {
                Node *next = _activeNode->children[_activeEdge];

                // character _text[step] IS in the active point?

                if (walkDown(next))
                    continue;

                if (_text[next->start + _activeLength] == _text[_pos])
                {
                    if (lastSplitNode != nullptr && _activeNode != _root)
                    {
                        lastSplitNode->suffixLink = _activeNode;
                        lastSplitNode = nullptr;
                    }

                    // Let's move the active point one position to the right!
                    _activeLength++;
                    break;
                }

                // character _text[step] IS NOT in the active point?

                // create a split node that will be between "activeNode" and node "next"

                int *splitEnd = new int;
                *splitEnd = next->start + _activeLength - 1;

                Node *splitNode = new Node(next->start, splitEnd);

                _activeNode->children[_activeEdge] = splitNode;

                next->start += _activeLength;

                splitNode->children[_text[next->start] - 97] = next;

                // create a new node with label _text[step]. the new node will be son of "split"

                splitNode->children[nextChar] = new Node(step, &_pos);

                if (lastSplitNode != nullptr)
                    lastSplitNode->suffixLink = splitNode;

                lastSplitNode = splitNode;
            }

            _remainder--;

            if (_activeNode == _root && _activeLength > 0)
            {
                _activeLength--;
                _activeEdge =
                        _text[step - _remainder + 1] - 97; // move to the beggining of the suffix that "remains" to add
            } else if (_activeNode != _root)
            {
                _activeNode = _activeNode->suffixLink;
            }
        }
    }

    /*** Task: Check if innerPattern is in the text stored by the suffix tree. If the innerPattern exists, this function returns the position of the innerPattern in the text.
                If the innerPattern is not in the text, it returns -1 **/
    int findInnerPattern(std::string innerPattern, Node *startNode)
    {

        Node *crrNode = startNode;
        for (Node *child: crrNode->children)
            if (child)
            {
                auto edgeLength = child->edgeLength();
                if (matchPattern(innerPattern, child)) // the edge contains a part of the innerPattern
                {
//               end of the innerPattern
                    if (innerPattern.size() > edgeLength) // continue search
                        return findInnerPattern(
                                        innerPattern.substr(edgeLength),
                                        child) == -1 ? -1 : child->start;
                    else
                    {
                        return child->start;
                    }
                } else
                    continue;
            }
        return -1;
    }

    bool matchPattern(std::string pattern, Node *node)
    {
        auto edgeLength = node->edgeLength();
        bool flag = pattern.size() > edgeLength;
        int shortest_length = (1 - flag) * pattern.size() + flag * edgeLength;

        for (int i = 0; i < shortest_length; i++)
        {
            if (pattern[i] != _text[i + node->start])
                return false;
        }
        return true;
    }

    int find(std::string pattern)
    {
//        int pos;
        return findInnerPattern(pattern, _root);
    }

private:

    int _pos;

    Node *_root;

    std::string _text;

    int _remainder;

    // active point
    Node *_activeNode;
    int _activeEdge;
    int _activeLength;
};

#endif