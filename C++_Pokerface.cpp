
/*
 *	C++ Code Sample
 *	
 *  Created on: Oct. 12, 2012
 *      Author: Enze Zhou
 */

#include <stdio.h>
#include <ctype.h>
#include <cmath>

namespace {

	//	These two elements limit the input scale.
	const int MAX_N = 1000000;	//	Max allowed input n, <= 1000000.
	const int MAX_HEIGHT = 20;	//	Max height of the segment tree. Since 2 ^ 20 > 10 ^ 6,
								//	the max height of a binary tree should be 21, and the subscript should be 20.

	/*
	 *	This class is used as tree node for solving Pokeface problem.
	 *	Variables:
	 *		numFaceDown
	 *			The number of cards in this subtree that is face down.
	 *		middle
	 *			The index of the lower bound of the right child.
	 *		flipped
	 *			Whether information in this node is flipped.
	 *			If true, "numFaceDown" is the number of cards in this subtree that is face up.
	 *			At leaf node, "flipped" is used to record whether this card is face down.
	 *		left & right
	 *			Pointers to left and right child.
	 *	Methods:
	 *		treeNode
	 *			Default constructor.
	 */
	class treeNode{
	public:
		int     	numFaceDown, middle;
		bool    	flipped;
		treeNode*   left;
		treeNode*   right;
		treeNode ();
	};


	/*
	 * 	Set "numFaceDown" to 0 and "flipped" to false because at the beginning all the cards
	 *	are face up and none of the node is flipped.
	 */
	treeNode::treeNode (): numFaceDown(0), middle(-1), flipped(false), left(NULL), right(NULL) {}


	int treeHeight;						//	The subscript height of the tree, no more than 20.
	treeNode* root;						//	Root node of the segment tree being used.
	treeNode* path[2][MAX_HEIGHT + 1];	//	path[0] is path to the lower bound leaf node. path[0][0] is the leaf and path[0][treeHeight] is the root.
										//	and path[1] is path to the upper bound leaf node. path[1][0] is the leaf and path[1][treeHeight] is the root.

	/*
	 * 	twoPowers[i] = 2 ^ i, which is the size of the subtree if the node is at height i.
	 *	Store these numbers as constent here so that there's no need to calculate when using them.
	 */
	const int twoPowers[MAX_HEIGHT + 1] = {	1, 		2, 		4, 		8, 		16, 	32, 	64,
											128, 	256, 	512, 	1024, 	2048,	4096, 	8192,
											16384, 	32768, 	65536, 	131072, 262144, 524288, 1048576};


	/*
	 *	This function is called recursively to build the segment tree.
	 *	Parameters:
	 * 		height
	 * 			The height of the subtree that is being built.
	 * 		p
	 * 			The treeNode that is being built. & is necessary to pass result to caller.
	 * 		parMiddle
	 * 			"middle" element of parent. Used to determine the "middle" element of the new node.
	 * 		buildingLeft
	 * 			Whether this function is building the left subtree or the right subtree.
	 */
	void buildTree (const int height, treeNode*& p, const int parMiddle, const bool buildingLeft) {
		if (height == 0) { p = new treeNode; return; }
		p = new treeNode;
		p->middle = parMiddle + (buildingLeft ? ((-1) * twoPowers[height - 1]) : twoPowers[height - 1]);
		buildTree (height - 1, p->left, p->middle, true);
		buildTree (height - 1, p->right, p->middle, false);
	}


	/*
	 *	This function is overloaded, used to build a segment tree with input scale n.
	 *	Parameters:
	 *		n
	 *			Number of cards on the table.
	 */
	void buildTree (const int n) {
		if (n > MAX_N) printf("Error: n is too big.");

		//	Notice: element 0 is not used, so a tree whose max index is bigger
		//	than n must have volume of n + 1.
		int treeN = n + 1;

		root = new treeNode;
		treeHeight = (int)ceil(log2(treeN));
		root->middle = twoPowers[treeHeight - 1];
		buildTree(treeHeight - 1, root->left, root->middle, true);
		buildTree(treeHeight - 1, root->right, root->middle, false);
	}


	/*
	 * 	This function is used to find the path to a leaf node with given index.
	 *	Parameters:
	 * 		index
	 * 			The index of the card whose path will be found.
	 * 		leftPath
	 * 			Whether this function is building the path to lower bound or upper bound.
	 */
	void findPath (const int index, const int pathID) {
		treeNode* p = root;
		for (int i = 0; i < treeHeight; i++) {
			path[pathID][treeHeight - i] = p;
			if (index < p->middle) p = p->left;
			else p = p->right;
		}
		path[pathID][0] = p;
	}


	/*
	 *	This function is used to find the height where two paths separate.
	 *	Return value:
	 *		The height where two paths separate. If the two paths are identical, return -1.
	 */
	int findFork () {
	    int i = treeHeight;
	    while (i > -1 && path[0][i] == path[1][i]) --i;
	    return i;
	}


	/*
	 *	This function is used to flip the "flipped" element of a tree node.
	 *	Parameters:
	 * 		p
	 * 			The tree node whose "flipped" element needs to be flipped.
	 */
	void flip (treeNode* p) { p->flipped = !p->flipped; }


	/*
	 * 	Calculate the number of cards face down in a node, which is the sum of the same element
	 * 	modified by "flipped" sign in its two subtrees .
	 * 	Parameters:
	 * 		p
	 * 			The tree node whose "numFaceDown" element is being calculated.
	 * 		height
	 * 			The height of tree node p.
	 */
	void calNumFaceDown (treeNode* p, const int height) {
	    int numFaceDownL = (p->left->flipped ? twoPowers[height - 1] - p->left->numFaceDown : p->left->numFaceDown);
		int numFaceDownR = (p->right->flipped ? twoPowers[height - 1] - p->right->numFaceDown : p->right->numFaceDown);
	    p->numFaceDown = numFaceDownL + numFaceDownR;
	}


	/*
	 * 	This function handles a wave hand action from input and modify the segment tree.
	 * 	Parameters:
	 * 		l
	 * 			The lower bound of the index of cards being flipped.
	 * 		r
	 * 			The upper bound of the index of cards being flipped.
	 */
	void waveHand (const int l, const int r) {

		//	Look for the paths from root to boundary leaves.
		findPath (l, 0);
		findPath (r, 1);

		//	Handle "flipped" signs along the two paths. "numFaceDown" elements don't need
		//	to be handled here because they will be updated soon.
	    for (int i = treeHeight; i > 0; i--)
	        if (path[0][i]->flipped){
	        	flip (path[0][i]->left);
	        	flip (path[0][i]->right);
	            path[0][i]->flipped = false;
	        }
	    int fork = findFork ();
	    for (int i = fork; i > 0; i--)
	        if (path[1][i]->flipped) {
	        	flip (path[1][i]->left);
	        	flip (path[1][i]->right);
	            path[1][i]->flipped = false;
	        }

	    //	fork == -1 means that the two paths are identical, so we only need to flip one card
	    //	and update "numFaceDown" along this path.
	    if (fork == -1) {
	    	flip (path[0][0]);
	    	for (int i = 1; i <= treeHeight; i++)
	    		calNumFaceDown (path[0][i], i);
	        return;
	    }

	    //	Flip cards in the interval.
	    //	If all the cards in a subtree is flipped, just change its "flipped" sign to accelerate process.
	    flip (path[0][0]);
	    flip (path[1][0]);
	    for (int i = 0; i < fork; ++i) {
	    	if (path[0][i] == path[0][i + 1]->left)
	    		flip (path[0][i + 1]->right);
	    	if (path[1][i] == path[1][i + 1]->right)
	    		flip (path[1][i + 1]->left);
	    }

	    //	Update "numFaceDown" along the paths.
	    for (int i = 1; i <= fork; ++i) {
	    	calNumFaceDown (path[0][i], i);
	    	calNumFaceDown (path[1][i], i);
	    }
    	for (int i = fork + 1; i <= treeHeight; i++)
    		calNumFaceDown (path[0][i], i);
	}	//	waveHand


	/*
	 * 	This function handles a request.
	 * 	Parameters:
	 * 		l
	 * 			The lower bound of the asked index interval.
	 * 		r
	 * 			The upper bound of the asked index interval.
	 * 	Return value:
	 * 		The number of card that are face up in the asked index interval.
	 */
	int answerAud (const int l, const int r) {

		//	Look for the paths from root to boundary leaves.
		findPath (l, 0);
		findPath (r, 1);

		//	Handle "flipped" signs along the two paths.
		int fork = findFork ();
		for (int i = treeHeight; i > 0; i--)
			if (path[0][i]->flipped) {
				flip (path[0][i]->right);
				flip (path[0][i]->left);
				path[0][i]->numFaceDown = twoPowers[i] - path[0][i]->numFaceDown;
				path[0][i]->flipped = false;
			}
	    for (int i = fork; i > 0; i--)
	        if (path[1][i]->flipped){
	        	flip (path[1][i]->left);
	        	flip (path[1][i]->right);
	        	path[1][i]->numFaceDown = twoPowers[i] - path[1][i]->numFaceDown;
	            path[1][i]->flipped = false;
	        }

	    //	Handle some specific circumstances where only 1 or 2 cards are asked.
		if (fork == -1)
			return (path[0][0]->flipped ? 0 : 1);
	    else if (fork == 0) {
	        int ans = 0;
	        if (!path[0][0]->flipped) ans++;
	        if (!path[1][0]->flipped) ans++;
	        return ans;
	    }

		//	General circumstance, where we need to sum the number of face-up cards in this interval.
	    int ans = 0;
	    if (!path[0][0]->flipped) ans++;	//	Two boundary cards.
	    if (!path[1][0]->flipped) ans++;
	    //	Accumulate cards surrounded by left boundary after the paths separate.
	    for (int i = 0; i < fork; i++)
	        if (path[0][i] == path[0][i + 1]->left) {
	            int temp = path[0][i + 1]->right->flipped ? path[0][i + 1]->right->numFaceDown : twoPowers[i] - path[0][i + 1]->right->numFaceDown;
	            ans += temp;
	        }
	    //	Accumulate cards surrounded by right boundary after the paths separate.
	    for (int i = 0; i < fork; i++)
	        if (path[1][i] == path[1][i + 1]->right) {
	            int temp = path[1][i + 1]->left->flipped ? path[1][i + 1]->left->numFaceDown : twoPowers[i] - path[1][i + 1]->left->numFaceDown;
	            ans += temp;
	        }
	    return ans;
	}	//	answerAud

}	//	Anonymous namespace


int main () {

	//	n: number of cards; h: number of wave hand action; 	q: number of questions;
	//	l: lower bound; 	r: upper bound;
	int n, h, q, l, r;
	scanf("%d %d %d",&n,&h,&q);

	buildTree(n);
	char input = getchar();
	for (int i = 0; i < h + q; i++) {
        while(!isalpha(input))
        	input = getchar();
        if(input == 'H') {
            scanf("%d %d", &l, &r);
            waveHand(l, r);
        } else if (input == 'Q') {
            scanf("%d %d", &l, &r);
            printf("%d\n", answerAud(l, r));
        }
        input=getchar();
	}
    return 0;
}	//	main




