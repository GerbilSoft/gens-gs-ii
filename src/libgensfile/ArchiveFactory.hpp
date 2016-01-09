/***************************************************************************
 * libgensfile: Gens file handling library.                                *
 * ArchiveFactory.hpp: Archive factory class.                              *
 *                                                                         *
 * Copyright (c) 2015-2016 by David Korth.                                 *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; either version 2 of the License, or (at your  *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.           *
 ***************************************************************************/

#ifndef __LIBGENSFILE_ARCHIVEFACTORY_HPP__
#define __LIBGENSFILE_ARCHIVEFACTORY_HPP__

namespace LibGensFile {

class Archive;

class ArchiveFactory
{
	private:
		// Static class.
		ArchiveFactory() { }
		~ArchiveFactory() { }

	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibGens-specific version of Q_DISABLE_COPY().
		ArchiveFactory(const ArchiveFactory &);
		ArchiveFactory &operator=(const ArchiveFactory &);

	public:
		/**
		 * Open the specified file using an Archive subclass.
		 * @param filename Archive filename.
		 * @return Opened archive, or nullptr on error. (TODO: Return an error code?)
		 */
		static Archive *openArchive(const char *filename);
};

}

#endif /* __LIBGENSFILE_ARCHIVEFACTORY_HPP__ */
