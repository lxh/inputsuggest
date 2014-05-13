
#include "isindex/TSTPat.h"

int CTSTPat::alloc_nums()
{
	return ptree_node_manager_->used_unit_num();
}
CTSTPat::CTSTPat():	ptree_node_manager_(NULL)
{
}

CTSTPat::~CTSTPat()
{
	if(ptree_node_manager_)
		delete ptree_node_manager_;
}

int CTSTPat::init(int nmax_tree_block_num, int ntree_block_unit_num)
{
	if(ptree_node_manager_)
		delete ptree_node_manager_;
	ptree_node_manager_ = new CUnitMemoryManager();
	ptree_node_manager_->init(nmax_tree_block_num, ntree_block_unit_num, sizeof(CTreeNode));

	return 0;
}

/**************************************** left_rotate *******************************************
功能：节点左旋操作
返回值：
参数：
	CTreeNode		*&proot		out		层树根指针引用
	CTreeNode		*pnode		in		旋转节点
程序员：
创建日期：
最后修改日期：
备注：
************************************************************************************************/
void CTSTPat::left_rotate(CTreeNode *&proot, CTreeNode *pnode)
{
	CTreeNode *pright = pnode->pright_;

	pnode->pright_ = pright->pleft_;
	if(pnode->pright_)
		pnode->pright_->pparent_ = pnode;
	if(!pnode->pparent_)
	{
		proot = pright;
	}
	else if(pnode == pnode->pparent_->pleft_)
	{
		pnode->pparent_->pleft_ = pright;
	}
	else
	{
		pnode->pparent_->pright_ = pright;
	}
	pright->pparent_ = pnode->pparent_;
	pright->pleft_ = pnode;
	pnode->pparent_ = pright;
}

/**************************************** right_rotate ******************************************
功能：节点右旋操作
返回值：
参数：
	CTreeNode		*&proot		out		层树根指针引用
	CTreeNode		*pnode		in		旋转节点
程序员：
创建日期：
最后修改日期：
备注：
************************************************************************************************/
void CTSTPat::right_rotate(CTreeNode *&proot, CTreeNode *pnode)
{
	CTreeNode *pleft = pnode->pleft_;

	pnode->pleft_ = pleft->pright_;
	if(pnode->pleft_)
		pnode->pleft_->pparent_ = pnode;
	if(!pnode->pparent_)
	{
		proot = pleft;
	}
	else if(pnode == pnode->pparent_->pleft_)
	{
		pnode->pparent_->pleft_ = pleft;
	}
	else
	{
		pnode->pparent_->pright_ = pleft;
	}
	pleft->pparent_ = pnode->pparent_;
	pleft->pright_ = pnode;
	pnode->pparent_ = pleft;
}

/**************************************** add_fixup *********************************************
功能：对同层内部节点作层增加节点的调整操作
返回值：成功返回0,失败返回-1
参数：
	CTreeNode		*&proot		out		层树根指针引用
	CTreeNode		*pnode		in		增加节点
程序员：
创建日期：
最后修改日期：
备注：
************************************************************************************************/
int CTSTPat::add_fixup(CTreeNode *&proot, CTreeNode *pnode)
{
	CTreeNode *p, *pparent_right, *pparent_left;

	p = pnode;
	while(p->pparent_ && p->pparent_->ucolor_ == RED)
	{
		if(p->pparent_ == p->pparent_->pparent_->pleft_)
		{
			pparent_right = p->pparent_->pparent_->pright_;
			if(pparent_right && pparent_right->ucolor_ == RED)
			{
				p->pparent_->ucolor_ = BLACK;
				pparent_right->ucolor_ = BLACK;
				p->pparent_->pparent_->ucolor_ = RED;
				p = p->pparent_->pparent_;
			}
			else
			{
				if(p == p->pparent_->pright_)
				{
					p = p->pparent_;
					left_rotate(proot, p);
				}
				p->pparent_->ucolor_ = BLACK;
				p->pparent_->pparent_->ucolor_ = RED;
				right_rotate(proot, p->pparent_->pparent_);
			}
		}
		else
		{
			pparent_left = p->pparent_->pparent_->pleft_;
			if(pparent_left && pparent_left->ucolor_ == RED)
			{
				p->pparent_->ucolor_ = BLACK;
				pparent_left->ucolor_ = BLACK;
				p->pparent_->pparent_->ucolor_ = RED;
				p = p->pparent_->pparent_;
			}
			else
			{
				if(p == p->pparent_->pleft_)
				{
					p = p->pparent_;
					right_rotate(proot, p);
				}
				p->pparent_->ucolor_ = BLACK;
				p->pparent_->pparent_->ucolor_ = RED;
				left_rotate(proot, p->pparent_->pparent_);
			}
		}
	}
	proot->ucolor_ = BLACK;

	return 0;
}

/***************************************** insert ***********************************************
功能：将数据项插入树中
返回值：成功返回0，失败返回-1，内存不足返回1
参数：
	CTreeNode			*&proot		in		前缀树根
	CLeafNode			*pitem		in		数据项指针
程序员：
创建日期：
最后修改日期：
备注：
************************************************************************************************/
long CTSTPat::insert(CTreeNode *&proot, CLeafNode *pitem)
{
	CTreeNode *p, *pnext, **pproot;
	long i;
	char c, c2;

	if(pitem->is_null())
		return -1;

	//头节点
	if(!proot)
	{
		proot = (CTreeNode *)ptree_node_manager_->alloc();
		if(!proot)
			return 1;
		proot->set_node(0, pitem->get_byte(0), NULL, NULL, BLACK, pitem);

		return 0;
	}

	//找见插入位置
	p = proot;
	pnext = proot;
	while(pnext)
	{
		c = pitem->get_byte(pnext->sbyte_index_);
		if(c < pnext->cbyte_)
		{
			p = pnext;
			pnext = pnext->pleft_;
		}
		else if(c == pnext->cbyte_)
		{
			if(pnext->pmiddle_)
			{
				p = pnext;
				pnext = pnext->pmiddle_;
			}
			else
			{
				if((*(pnext->pitem_)) == *pitem)
					return -1;
				else
					break;
			}
		}
		else
		{
			p = pnext;
			pnext = pnext->pright_;
		}
	}

	//获取不同的byte位置
	if(pnext)
	{
		i = pitem->get_first_different_byte(pnext->pitem_, 0);
		c2 = pnext->pitem_->get_byte(i);
	}
	else
	{
		while(p->pmiddle_)
			p = p->pmiddle_;
		i = pitem->get_first_different_byte(p->pitem_, 0);
		c2 = p->pitem_->get_byte(i);
	}

	p = proot;
	pnext = proot;
	pproot = &proot;
	//
	while(pnext && pnext->sbyte_index_ <= i)
	{
		c = pitem->get_byte(pnext->sbyte_index_);
		if(c < pnext->cbyte_)
		{
			p = pnext;
			pnext = pnext->pleft_;
		}
		else if(c == pnext->cbyte_)
		{
			if(pnext->pmiddle_)
			{
				p = pnext;
				pproot = &pnext->pmiddle_;
				pnext = pnext->pmiddle_;
			}
			else
			{
				p = pnext;
				break;
			}
		}
		else
		{
			p = pnext;
			pnext = pnext->pright_;
		}
	}
	//插入节点
	if(pnext)
	{
		p->pmiddle_ = (CTreeNode *)ptree_node_manager_->alloc();
		if(!p->pmiddle_)
			return 1;
		if(p == pnext)
		{
			p->pmiddle_->set_node((short)i, c2, NULL, NULL, BLACK, p->pitem_);
			p->pitem_ = NULL;
		}
		else
		{
			p->pmiddle_->set_node((short)i, c2, pnext, NULL, BLACK, NULL);
		}

		c = pitem->get_byte(i);
		if(c < c2)
		{
			p->pmiddle_->pleft_ = (CTreeNode *)ptree_node_manager_->alloc();
			if(!p->pmiddle_->pleft_)
				return 1;
			p->pmiddle_->pleft_->set_node((short)i, c, NULL, p->pmiddle_, RED, pitem);
		}
		else
		{
			p->pmiddle_->pright_ = (CTreeNode *)ptree_node_manager_->alloc();
			if(!p->pmiddle_->pright_)
				return 1;
			p->pmiddle_->pright_->set_node((short)i, c, NULL, p->pmiddle_, RED, pitem);
		}
	}
	else
	{
		if(c < p->cbyte_)
		{
			p->pleft_ = (CTreeNode *)ptree_node_manager_->alloc();
			if(!p->pleft_)
				return 1;
			p->pleft_->set_node(p->sbyte_index_, c, NULL, p, RED, pitem);
			add_fixup(*pproot, p->pleft_);
		}
		else
		{
			p->pright_ = (CTreeNode *)ptree_node_manager_->alloc();
			if(!p->pright_)
				return 1;
			p->pright_->set_node(p->sbyte_index_, c, NULL, p, RED, pitem);
			add_fixup(*pproot, p->pright_);
		}
	}

	return 0;
}

/****************************************** search **********************************************
功能：精确检索pitem数据
返回值：成功返回对应叶子节点，失败返回NULL
参数：
	CTreeNode		*proot		in		前缀树根
	CLeafNode		*pitem		in		检索数据
程序员：
创建日期：
最后修改日期：
备注：
************************************************************************************************/
CLeafNode *CTSTPat::search(CTreeNode *proot, CLeafNode *pitem)
{
	CTreeNode *p;
	char c;

	if(pitem->is_null() || !proot)
		return NULL;

	p = proot;
	while(p)
	{
		c = pitem->get_byte(p->sbyte_index_);		//这里可以优化
		if(c < p->cbyte_)
			p = p->pleft_;
		else if(c == p->cbyte_)
		{
			if(p->pmiddle_)
			{
				p = p->pmiddle_;
			}
			else
			{
				if(*(p->pitem_) == *pitem)
					return p->pitem_;
				else
					break;
			}
		}
		else
			p = p->pright_;
	}

	return NULL;
}

int CTSTPat::del_fixup(CTreeNode *&proot, CTreeNode *pnode)
{
	CTreeNode *psibling, *pdel;

	while(pnode != proot && pnode->ucolor_ == BLACK)
	{//双黑
		if(pnode == pnode->pparent_->pleft_)
		{
			psibling = pnode->pparent_->pright_;
			if(psibling->ucolor_ == RED)
			{
				psibling->ucolor_ = BLACK;
				pnode->pparent_->ucolor_ = RED;
				left_rotate(proot, pnode->pparent_);
				psibling = pnode->pparent_->pright_;
			}
			//
			if(-1 == pnode->sbyte_index_)
			{
				if(!psibling->pleft_ && !psibling->pright_)
				{
					psibling->ucolor_ = RED;
					pdel = pnode;
					pnode = pnode->pparent_;
					pnode->pleft_ = NULL;
					ptree_node_manager_->free((void *)pdel);
				}
				else
				{
					if(!psibling->pright_)
					{
						psibling->pleft_->ucolor_ = BLACK;
						psibling->ucolor_ = RED;
						right_rotate(proot, psibling);
						psibling = psibling->pparent_;
					}
					psibling->ucolor_ = psibling->pparent_->ucolor_;
					psibling->pparent_->ucolor_ = BLACK;
					psibling->pparent_->pleft_ = NULL;
					psibling->pright_->ucolor_ = BLACK;
					left_rotate(proot, psibling->pparent_);
					ptree_node_manager_->free((void *)pnode);
					pnode = proot;
				}
			}
			else
			{
				if(psibling->pleft_->ucolor_ == BLACK && psibling->pright_->ucolor_ == BLACK)
				{
					psibling->ucolor_ = RED;
					pnode = pnode->pparent_;
				}
				else
				{
					if(psibling->pright_->ucolor_ == BLACK)
					{
						psibling->pleft_->ucolor_ = BLACK;
						psibling->ucolor_ = RED;
						right_rotate(proot, psibling);
						psibling = psibling->pparent_;
					}
					psibling->ucolor_ = psibling->pparent_->ucolor_;
					psibling->pparent_->ucolor_ = BLACK;
					psibling->pright_->ucolor_ = BLACK;
					left_rotate(proot, psibling->pparent_);
					pnode = proot;
				}
			}
		}
		else
		{
			psibling = pnode->pparent_->pleft_;
			if(psibling->ucolor_ == RED)
			{
				psibling->ucolor_ = BLACK;
				pnode->pparent_->ucolor_ = RED;
				right_rotate(proot, pnode->pparent_);
				psibling = pnode->pparent_->pleft_;
			}
			//
			if(-1 == pnode->sbyte_index_)
			{
				if(!psibling->pleft_ && !psibling->pright_)
				{
					psibling->ucolor_ = RED;
					pdel = pnode;
					pnode = pnode->pparent_;
					pnode->pright_ = NULL;
					ptree_node_manager_->free((void *)pdel);
				}
				else
				{
					if(!psibling->pleft_)
					{
						psibling->pright_->ucolor_ = BLACK;
						psibling->ucolor_ = RED;
						left_rotate(proot, psibling);
						psibling = psibling->pparent_;
					}
					psibling->ucolor_ = psibling->pparent_->ucolor_;
					psibling->pparent_->ucolor_ = BLACK;
					psibling->pparent_->pright_ = NULL;
					psibling->pleft_->ucolor_ = BLACK;
					right_rotate(proot, psibling->pparent_);
					ptree_node_manager_->free((void *)pnode);
					pnode = proot;
				}
			}
			else
			{
				if(psibling->pleft_->ucolor_ == BLACK && psibling->pright_->ucolor_ == BLACK)
				{
					psibling->ucolor_ = RED;
					pnode = pnode->pparent_;
				}
				else
				{
					if(psibling->pleft_->ucolor_ == BLACK)
					{
						psibling->pright_->ucolor_ = BLACK;
						psibling->ucolor_ = RED;
						left_rotate(proot, psibling);
						psibling = psibling->pparent_;
					}
					psibling->ucolor_ = psibling->pparent_->ucolor_;
					psibling->pparent_->ucolor_ = BLACK;
					psibling->pleft_->ucolor_ = BLACK;
					right_rotate(proot, psibling->pparent_);
					pnode = proot;
				}
			}
		}
	}
	pnode->ucolor_ = BLACK;

	return 0;
}

CTreeNode *CTSTPat::tree_min(CTreeNode *proot)
{
	if(!proot)
		return NULL;

	while(proot->pleft_)
		proot = proot->pleft_;

	return proot;
}

CLeafNode* CTSTPat::remove(CTreeNode *&proot, CLeafNode *pitem)
{
	CTreeNode *p, *ppre_level, **pproot, *pdel, *preplace;
	char c;
	CLeafNode *pres;

	if(!pitem || pitem->is_null() || !proot)
		return NULL;

	pres = NULL;
	p = proot;
	ppre_level = proot;
	pproot = &proot;
	while(p)
	{
		c = pitem->get_byte(p->sbyte_index_);
		if(c < p->cbyte_)
		{
			p = p->pleft_;
		}
		else if(c == p->cbyte_)
		{
			if(p->pmiddle_)
			{
				ppre_level = p;
				pproot = &p->pmiddle_;
				p = p->pmiddle_;
			}
			else
			{
				if(*(p->pitem_) == *pitem)
					break;
				else
					return NULL;
			}
		}
		else
		{
			p = p->pright_;
		}
	}
	if(!p)
		return NULL;
	pres = p->pitem_;
	//
	if(p->pleft_ && p->pright_)
		pdel = tree_min(p->pright_);
	else
		pdel = p;
	//
	preplace = pdel->pleft_ ? pdel->pleft_ : pdel->pright_;
	if(preplace)
	{
		preplace->pparent_ = pdel->pparent_;
		if(preplace->pparent_)
		{
			if(pdel == pdel->pparent_->pleft_)
				pdel->pparent_->pleft_ = preplace;
			else
				pdel->pparent_->pright_ = preplace;
		}
		else
			*pproot = preplace;
	}
	else
	{
		preplace = pdel;
		preplace->sbyte_index_ = -1;
	}
	//
	if(pdel != p)
	{
		p->cbyte_ = pdel->cbyte_;
		p->pmiddle_ = pdel->pmiddle_;
		p->pitem_ = pdel->pitem_;
	}
	//
	if(pdel->ucolor_ == BLACK)
		del_fixup(*pproot, preplace);

	if(preplace != pdel)
		ptree_node_manager_->free((void *)pdel);
	else if(pdel->ucolor_ == RED)
	{
		if(pdel == pdel->pparent_->pleft_)
			pdel->pparent_->pleft_ = NULL;
		else
			pdel->pparent_->pright_ = NULL;
		ptree_node_manager_->free((void *)pdel);
	}

	if(!((*pproot)->pleft_) && !((*pproot)->pright_) && *pproot != proot)
	{
		CTreeNode *pd = *pproot;
		ppre_level->pitem_ = (*pproot)->pitem_;
		ppre_level->pmiddle_ = (*pproot)->pmiddle_;
		ptree_node_manager_->free((void *)pd);
	}

	return pres;
}

void CTSTPat::clear()
{
	ptree_node_manager_->clear();
}
