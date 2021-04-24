/**
 * \file block.cpp
 * \brief Implémentation d'un bloc.
 * \author Maxime Miville Deschenes, Philippe Vincent, Francis Boulianne
 * \version 0.1
 * \date  2021
 *
 *  Travail pratique numéro 3
 *
 */

#include "block.h"
//vous pouvez inclure d'autres librairies si c'est nécessaire

namespace TP3
{

	// déstructeur de l'objet Block
	Block::~Block()
	{

		// on détruit  les détruits vecteurs m_dirEntry et les vecteurs m_bitmap
		m_dirEntry.clear();
		m_bitmap.clear();
		// on met m_type_donnes à 0 pour ne pas qu'ils soient un type
		m_type_donnees = 0;
		// on met le iNode à null
	}

	// Ajouter votre code ici !
	Block::Block(size_t td)
	{
		m_type_donnees = td;

		if (td == S_IFIL || td == S_IFBL) {
            m_bitmap.resize(128);
        }

		if (td == S_IFDE) {
		   // m_dirEntry.resize(28);
		}

	}

	Block::Block() {
    }

} //Fin du namespace
