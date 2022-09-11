#include "stdafx.h"
#include "psdl.h"
#include "resource.h"
#include "toolwnd.h"

#include <ctime>
#include <fstream>
#include <sstream>
#include <streambuf>

using namespace std;

psdl::attribute* psdl::block::get_attribute(unsigned long i_pos)
{
	if (i_pos < _attributes.size())
		return _attributes[i_pos];
	return 0;
}

error::code psdl::read_file(const char* filename, ProgressMonitor* monitor)
{
	//	clock_t start_time = clock();

	unsigned long i, n_size;

	ifstream f(filename, ios::in | ios::binary);

	if (!f.is_open()) return error::cant_open;

	ATLTRACE("\nReading file: %s\n", filename);

	f.seekg(0, ios_base::end);
	int fsize = f.tellg();
	f.seekg(0, ios_base::beg);

	monitor->setMaximum(fsize);

	//	char identifier[4];
	f.read(identifier, 4);

	unsigned char version = 0;

	if (strncmp(identifier, "PSD1", 4) == 0)
		version = 1;
	else if (strncmp(identifier, "PSD0", 4) != 0)
		return error::wrong_format;

	f.seekg(4, ios_base::cur);

	//	callback(_T("Reading vertices"), f.tellg() / fsize);
	monitor->setNote("Reading vertices");
	f.read((char*)&n_size, 4);
	ATLTRACE("Number of vertices: 0x%x\n", n_size);
	_vertices.reserve(n_size);

	Vertex vert;
	for (i = 0; i < n_size; ++i)
	{
		//	callback(_T(""), f.tellg() / fsize);

		f.read((char*)&vert, 12);
		add_vertex(vert);
		//	vertices.push_back(vertex);
		//	vertexRefs.push_back(i);

		//	ATLTRACE("Vertex %x: %f, %f, %f\n", i, vertexRefs[i]->x, vertexRefs[i]->y, vertexRefs[i]->z);
		monitor->setProgress(f.tellg());
	}

	monitor->setNote("Reading heights");

	//	callback(_T("Reading heights"), f.tellg() / fsize);
	f.read((char*)&n_size, 4);
	ATLTRACE("Number of heights: 0x%x\n", n_size);
	_heights.reserve(n_size);

	float height;
	for (i = 0; i < n_size; ++i)
	{
		//	callback(_T(""), f.tellg() / fsize);

		f.read((char*)&height, 4);
		add_height(height);
		monitor->setProgress(f.tellg());
	}

	monitor->setNote("Reading textures");

	//	callback(_T("Reading textures"), f.tellg() / fsize);
	/*
		if (version == 1) // Midnight Club 1
		{
			unsigned long n_textures;
			f.read((char*) &n_textures, 4);
			ATLTRACE("Number of textures: 0x%x\n", n_textures - 1);
			return error::ok;
		}
		else // Midtown Madness 2
		{*/
	f.read((char*)&n_size, 4);
	ATLTRACE("Number of textures: 0x%x\n", n_size - 1);
	_textures.reserve(n_size);

	vector<string> mtl;

	unsigned char n_length;
	for (i = 0; i < n_size - 1; ++i)
	{
		//	callback(_T(""), f.tellg() / fsize);

		f.read((char*)&n_length, 1);
		char* buffer = "";

		if (n_length > 0)
		{
			buffer = new char[n_length];
			f.read(buffer, n_length);
		}

		string textureName(buffer);

		{
			size_t found = textureName.find("lr");
			if (found != string::npos && found == 0)
			{
				//	textureName = "t" + textureName;
				mtl.push_back(textureName);
			}
		}

		//	ATLTRACE("%s ", textureName);
		add_texname(textureName.c_str());

		if (version == 1) f.seekg(1, ios_base::cur); // Midnight Club 1

		//	ATLTRACE("Texture %x: %s\n", i, textureName);
		monitor->setProgress(f.tellg());
	}

	sort(mtl.begin(), mtl.end());
	mtl.erase(unique(mtl.begin(), mtl.end()), mtl.end());

	for (vector<string>::iterator it = mtl.begin(); it != mtl.end(); ++it)
	{
		ATLTRACE("%s,cobblestone\n", (*it).c_str());
		//	ATLTRACE("%s\t\t\t\t1\t\t\t0\t\t\t1\n", (*it).c_str());
	}

	//	}

	monitor->setNote("Reading blocks");
	//	callback(_T("Reading blocks"), f.tellg() / fsize);
	f.read((char*)&n_size, 4);
	f.read((char*)&_unknown0, 4);
	n_size--;
	ATLTRACE("Number of blocks: 0x%x\n", n_size);
	_blocks.reserve(n_size);

	//	vector<perimeter_pt_i>* perimeter_data = new vector<perimeter_pt_i>[n_size];

	for (i = 0; i < n_size; ++i)
	{
		//	callback(_T(""), f.tellg() / fsize);

		psdl::block* block = new psdl::block(&_vertices);
		unsigned long j, n_perimeters, n_attributesize;

		f.read((char*)&n_perimeters, 4);
		f.read((char*)&n_attributesize, 4);

		//	perimeter_data[i].reserve(n_perimeters);
		block->_perimeter.reserve(n_perimeters);

		for (j = 0; j < n_perimeters; ++j)
		{
			perimeter_pt pp;

			//	f.read((char*) &pp.vertex, 2);
			//	f.read((char*) &pp.block,  2);

			//	unsigned short i_vertex;
			//	unsigned short i_block;

			//	f.read((char*) &i_vertex, 2);
			//	f.read((char*) &i_block,  2);

			f.read((char*)&pp, sizeof(perimeter_pt));

			block->add_perimeter_point(pp);
			//	block->add_perimeter_point(get_vertex(i_vertex), i_block > 0 ? _blocks.begin() + i_block : NULL);
		}

		unsigned int nAttributes = 0;

		long targetPos = (long)f.tellg() + (2 * n_attributesize);

		while (f.tellg() < targetPos)
		{
			u_short id;
			unsigned char type, subtype;
			bool last;

			f.read((char*)&id, 2);

			//	ATLTRACE("Attribute %x: %x\n", nAttributes, id);
												 // ---- masks ----
			//	last    = (id & 0x80) >> 7 == 1; // 0x80 = 10000000
			//	type    = (id & 0x78) >> 3;      // 0x78 = 01111000
			//	subtype = (id & 0x07);           // 0x07 = 00000111

			last = id >> 7 & 0x1 == 1;    // 0x1  = 00000001
			type = id >> 3 & 0xf;         // 0xf  = 00001111
			subtype = id & 0x7;         // 0x7  = 00000111

			if (type > 0xc) // no such attribute!
			{
				f.seekg(targetPos); // set file position to end of attribute list
				break; // exit while loop
			}

			attribute* atb = NULL;

			switch (type)
			{
			case 0x0:
				atb = new road_strip(&_vertices);
				{
					unsigned short k, nSections, vertexRef;

					if (subtype)
						nSections = subtype;
					else
						f.read((char*)&nSections, 2);

					for (k = 0; k < 4 * nSections; ++k)
					{
						f.read((char*)&vertexRef, 2);
						static_cast<road_strip*>(atb)->add_vertex(vertexRef);
						//static_cast<road_strip*>(atb)->add_vertex(get_vertex(vertexRef));
					}
				}
				break;

			case 0x1:
				atb = new sidewalk_strip(&_vertices);
				{
					unsigned short k, nSections, vertexRef;

					if (subtype)
						nSections = subtype;
					else
						f.read((char*)&nSections, 2);

					for (k = 0; k < 2 * nSections; ++k)
					{
						f.read((char*)&vertexRef, 2);
						static_cast<sidewalk_strip*>(atb)->add_vertex(vertexRef);
						//static_cast<sidewalk_strip*>(atb)->add_vertex(get_vertex(vertexRef));
					}
				}
				break;

			case 0x2:
				atb = new rectangle_strip(&_vertices);
				{
					unsigned short k, nSections, vertexRef;

					if (subtype)
						nSections = subtype;
					else
						f.read((char*)&nSections, 2);

					for (k = 0; k < 2 * nSections; ++k)
					{
						f.read((char*)&vertexRef, 2);
						static_cast<rectangle_strip*>(atb)->add_vertex(vertexRef);
						//static_cast<rectangle_strip*>(atb)->add_vertex(get_vertex(vertexRef));
					}
				}
				break;

			case 0x3:
				atb = new sliver(&_vertices);

				f.read((char*)&static_cast<sliver*>(atb)->top, 2);
				f.read((char*)&static_cast<sliver*>(atb)->tex_scale, 2);
				//	f.read((char*) &static_cast<sliver*>(atb)->left,      2);
				//	f.read((char*) &static_cast<sliver*>(atb)->right,     2);
				{
					unsigned short k, vertexRef;

					for (k = 0; k < 2; ++k)
					{
						f.read((char*)&vertexRef, 2);
						static_cast<sliver*>(atb)->set_vertex(k, vertexRef);
					}
				}
				break;

			case 0x4:
				atb = new crosswalk(&_vertices);
				{
					unsigned short k, vertexRef;

					for (k = 0; k < 4; ++k) // always 4 vertices
					{
						//	f.read((char*) &static_cast<crosswalk*>(atb)->_i_vertices[k], 2);
						f.read((char*)&vertexRef, 2);
						static_cast<crosswalk*>(atb)->set_vertex(k, vertexRef);
					}
				}
				break;

			case 0x5:
				atb = new road_triangle_fan(&_vertices);
				{
					unsigned short k, nTriangles, vertexRef;

					if (subtype)
						nTriangles = subtype;
					else
						f.read((char*)&nTriangles, 2);

					for (k = 0; k < nTriangles + 2; ++k)
					{
						f.read((char*)&vertexRef, 2);
						static_cast<road_triangle_fan*>(atb)->add_vertex(vertexRef);
						//static_cast<road_triangle_fan*>(atb)->add_vertex(get_vertex(vertexRef));
					}
				}
				break;

			case 0x6:
				atb = new triangle_fan(&_vertices);
				{
					unsigned short k, nTriangles, vertexRef;

					if (subtype)
						nTriangles = subtype;
					else
						f.read((char*)&nTriangles, 2);

					for (k = 0; k < nTriangles + 2; ++k) {
						f.read((char*)&vertexRef, 2);
						static_cast<triangle_fan*>(atb)->add_vertex(vertexRef);
						//static_cast<triangle_fan*>(atb)->add_vertex(get_vertex(vertexRef));
					}
				}
				break;

			case 0x7:
				atb = new facade_bound(&_vertices);
				f.read((char*)&static_cast<facade_bound*>(atb)->angle, 2);
				f.read((char*)&static_cast<facade_bound*>(atb)->top, 2);
				//	f.read((char*) &static_cast<facade_bound*>(atb)->left,	2);
				//	f.read((char*) &static_cast<facade_bound*>(atb)->right,	2);
				{
					unsigned short k, vertexRef;

					for (k = 0; k < 2; ++k)
					{
						f.read((char*)&vertexRef, 2);
						static_cast<facade_bound*>(atb)->set_vertex(k, vertexRef);
					}
				}
				break;

			case 0x8:
				atb = new divided_road_strip(&_vertices);
				{
					unsigned short k, nSections, vertexRef;

					if (subtype)
						nSections = subtype;
					else
						f.read((char*)&nSections, 2);

					f.read((char*)&static_cast<divided_road_strip*>(atb)->flags, 1);
					f.read((char*)&static_cast<divided_road_strip*>(atb)->i_texture, 1);
					f.read((char*)&static_cast<divided_road_strip*>(atb)->height, 1);
					f.read((char*)&static_cast<divided_road_strip*>(atb)->value, 1);

					static_cast<divided_road_strip*>(atb)->i_texture--;

					//	static_cast<divided_road_strip*>(atb)->divider_type =
					//		static_cast<divided_road_strip*>(atb)->flags & 7;

					for (k = 0; k < 6 * nSections; ++k) {
						f.read((char*)&vertexRef, 2);
						static_cast<divided_road_strip*>(atb)->add_vertex(vertexRef);
						//static_cast<divided_road_strip*>(atb)->add_vertex(get_vertex(vertexRef));
					}
				}
				break;

			case 0x9:
			{
				if (subtype) // subtype 2 = banktest.psdl !!
				{
					atb = new tunnel();

					f.read((char*)&static_cast<tunnel*>(atb)->flags, 2);
					//	f.get();
					f.read((char*)&static_cast<tunnel*>(atb)->unknown1, 1);
					f.read((char*)&static_cast<tunnel*>(atb)->height1, 1);

					if (subtype > 2)
					{
						//	f.get();
						f.read((char*)&static_cast<tunnel*>(atb)->unknown2, 1);
						f.read((char*)&static_cast<tunnel*>(atb)->height2, 1);
					}

					subtype = 3; // REMOVE!!!!
				}
				else
				{
					atb = new junction();
					unsigned short n_length;

					f.read((char*)&n_length, 2);
					f.read((char*)&static_cast<junction*>(atb)->flags, 2);
					//	f.get();
					f.read((char*)&static_cast<junction*>(atb)->unknown1, 1);
					f.read((char*)&static_cast<junction*>(atb)->height1, 1);
					//	f.get();
					f.read((char*)&static_cast<junction*>(atb)->unknown2, 1);
					f.read((char*)&static_cast<junction*>(atb)->height2, 1);
					f.read((char*)&static_cast<junction*>(atb)->unknown3, 2);

					static_cast<junction*>(atb)->unknown3 = 0x0000; // REMOVE!!! This value controls something about the ceilings!

					n_length = 2 * (n_length - 4);
					static_cast<junction*>(atb)->_enabled_walls.resize(n_length);
					f.read((char*)&static_cast<junction*>(atb)->_enabled_walls[0], n_length);
				}
			}
			break;

			case 0xa:
				atb = new texture();
				{
					unsigned short textureRef;
					f.read((char*)&textureRef, 2);
					static_cast<texture*>(atb)->i_texture = textureRef + (256 * subtype) - 1;
				}
				break;

			case 0xb:
				atb = new facade(&_vertices);
				f.read((char*)&static_cast<facade*>(atb)->bottom, 2);
				f.read((char*)&static_cast<facade*>(atb)->top, 2);
				f.read((char*)&static_cast<facade*>(atb)->u_repeat, 2);
				f.read((char*)&static_cast<facade*>(atb)->v_repeat, 2);
				//	f.read((char*) &static_cast<facade*>(atb)->left,     2);
				//	f.read((char*) &static_cast<facade*>(atb)->right,    2);
				{
					unsigned short k, vertexRef;

					for (k = 0; k < 2; ++k)
					{
						f.read((char*)&vertexRef, 2);
						static_cast<facade*>(atb)->set_vertex(k, vertexRef);
					}
				}
				break;

			case 0xc:
				atb = new roof_triangle_fan(&_vertices);
				{
					unsigned short k, nVertices, vertexRef;

					if (subtype)
						nVertices = subtype;
					else
						f.read((char*)&nVertices, 2);

					f.read((char*)&static_cast<roof_triangle_fan*>(atb)->i_height, 2);

					for (k = 0; k < nVertices + 1; ++k) {
						f.read((char*)&vertexRef, 2);
						static_cast<roof_triangle_fan*>(atb)->add_vertex(vertexRef);
						//static_cast<roof_triangle_fan*>(atb)->add_vertex(get_vertex(vertexRef));
					}
				}
				break;
			}

			if (atb)
			{
				atb->last = last;
				atb->type = type;
				atb->subtype = subtype;
				//	atb->psdl    = this;
				block->add_attribute(atb);
				++nAttributes;
			}
		}

		add_block(block);
		monitor->setProgress(f.tellg());
		//	ATLTRACE("Block %x: done!\n", i);
	}

	/*	// all vertices and blocks are read, so we can convert the perimeter indices to pointers:
		for (i = 0; i < n_size; ++i)
		{
			block* block = get_block(i);

			for (unsigned long j = 0; j < perimeter_data[i].size(); ++j)
			{
				perimeter_pt pp =
				{
					&_vertices[perimeter_data[i][j].i_vertex],
					_blocks   [perimeter_data[i][j].i_block]
				};

				block->add_perimeter_point(pp);
			}
		}*/

	_unknown1 = f.get();
	ATLTRACE("unknown1 = %x\n", _unknown1);

	for (i = 0; i < n_size; ++i)
	{
		f.read((char*)&_blocks[i]->type, 1);
		//	_blocks[i]->type &= 0x1e; // REMOVE!!!
	}

	_unknown2 = f.get();
	ATLTRACE("unknown2 = %x\n", _unknown2);

	for (i = 0; i < n_size; ++i)
	{
		f.read((char*)&_blocks[i]->proprule, 1);
	}

	if (version == 1) return error::ok;

	f.read((char*)&v_min, 12);
	f.read((char*)&v_max, 12);
	f.read((char*)&v_center, 12);
	f.read((char*)&f_radius, 4);

	unsigned char l;
	unsigned short j, nRoadBlock;
	float fDensity;

	f.read((char*)&n_size, 4);
	ATLTRACE("Number of blockpaths: 0x%x\n", n_size);
	_blockpaths.reserve(n_size);

	for (i = 0; i < n_size; ++i)
	{
		blockpath* path = new blockpath();;

		f.read((char*)&path->unknown4, 2);
		f.read((char*)&path->unknown5, 2);
		f.read((char*)&path->nFLanes, 1);
		f.read((char*)&path->nBLanes, 1);

		for (j = 0; j < path->nFLanes + path->nBLanes; ++j)
		{
			f.read((char*)&fDensity, 4);
			path->density.push_back(fDensity);
		}

		f.read((char*)&path->unknown6, 2);

		for (l = 0; l < 4; ++l)
			f.read((char*)&path->startCrossroads[l], 2);

		for (l = 0; l < 4; ++l)
			f.read((char*)&path->endCrossroads[l], 2);

		f.read((char*)&path->nRoadBlocks, 1);

		for (l = 0; l < path->nRoadBlocks; ++l)
		{
			f.read((char*)&nRoadBlock, 2);
			path->roadBlocks.push_back(nRoadBlock);
		}

		add_blockpath(path);
	}

	unsigned long nRemaining = 0;
	unsigned char data;

	while (data = f.get() != EOF)
	{
		_junk.push_back(data);
		nRemaining++;
	}

	ATLTRACE("Remaining bytes: 0x%x\n", nRemaining);

	//	unsigned long nRand = rand() % numVertices();
	//	ATLTRACE("Testing vertex %x: %f, %f, %f\n",
	//		nRand, getVertex(nRand).x, getVertex(nRand).y, getVertex(nRand).z);

	//	ATLTRACE("\nTime: %d", clock() - start_time);

	monitor->setProgress(f.tellg());
	return error::ok;
}

error::code psdl::write_file(const char* filename)
{
	unsigned long i = 0, n_size = 0;

	ofstream f(filename, ios::out | ios::binary);

	if (!f.is_open()) return error::cant_open;

	ATLTRACE("\nWriting file: %s\n", filename);

	f.write("PSD0", 4);
	unsigned long n_target_size = 2;
	f.write((char*)&n_target_size, 4);

	n_size = num_vertices();
	f.write((char*)&n_size, 4);

	for (i = 0; i < n_size; ++i)
	{
		f.write((char*)&_vertices[i], 12);
	}

	n_size = num_heights();
	f.write((char*)&n_size, 4);

	for (i = 0; i < n_size; ++i)
	{
		f.write((char*)&_heights[i], sizeof(float));
	}

	n_size = num_textures() + 1;
	f.write((char*)&n_size, 4);

	unsigned char n_length;
	for (i = 0; i < n_size - 1; ++i)
	{
		std::string texname = _textures[i];
		n_length = texname.length() + 1;

		if (n_length > 1)
		{
			f.write((char*)&n_length, 1);
			f.write((char*)texname.c_str(), n_length);
		}
		else
		{
			f.put(0);
		}
	}

	n_size = num_blocks() + 1;
	f.write((char*)&n_size, 4);
	f.write((char*)&_unknown0, 4);
	n_size--;

	for (i = 0; i < n_size; ++i)
	{
		block* block = _blocks[i];

		unsigned long j = 0, n_perimeters = 0, n_attributesize = 0, n_attributes = 0;

		n_perimeters = block->num_perimeters();
		f.write((char*)&n_perimeters, 4);

		//	n_attributesize = block->attributesize;
		streampos seek = f.tellp();
		f.write((char*)&n_attributesize, 4);

		for (j = 0; j < n_perimeters; ++j)
		{
			perimeter_pt pp = block->_perimeter[j];
			f.write((char*)&pp, sizeof(perimeter_pt));
		}

		n_attributes = block->num_attributes();
		//	if (n_attributes == 0) continue;
	/*
			if (block->type & (1 << 7))// REMOVES ATTRIBUTES FROM INST BLOCKS!!!
			{
				n_attributes = 0;
			}
	*/
	//	long targetPos = ftell(f) + 2 * nAttributeSize;
		j = 0;

		//	bool def_last = false;

		while (j < n_attributes)
		{
			unsigned short id = 0;
			unsigned char type = 0, subtype = 0;
			bool last = false;

			attribute* atb = block->_attributes[j];

			//	ATLTRACE("Block %x: writing attribute %x ...\n", i, j);

			//	last	 = (atb->last || j == n_attributes-1) ^ def_last;
			last = atb->last;
			type = atb->type;
			subtype = atb->subtype;

			//	def_last |= last;

			id = last << 7 | type << 3 | subtype;

			if (type == 0x9 && !subtype)
			{
				//	++j;
				//	continue;
			}
			else if (type == 0x8 && static_cast<divided_road_strip*>(atb)->num_sections() < 2)
			{
				++j;
				continue;
			}

			f.write((char*)&id, 2);
			/*
						switch (type)
						{
							case 0x0: static_cast<road_strip*>(atb)->write(f);         break;
							case 0x1: static_cast<sidewalk_strip*>(atb)->write(f);     break;
							case 0x2: static_cast<rectangle_strip*>(atb)->write(f);    break;
							case 0x3: static_cast<sliver*>(atb)->write(f);             break;
							case 0x4: static_cast<crosswalk*>(atb)->write(f);          break;
							case 0x5: static_cast<road_triangle_fan*>(atb)->write(f);  break;
							case 0x6: static_cast<triangle_fan*>(atb)->write(f);       break;
							case 0x7: static_cast<facade_bound*>(atb)->write(f);       break;
							case 0x8: static_cast<divided_road_strip*>(atb)->write(f); break;
							case 0x9:
								if (subtype)
									  static_cast<tunnel*>(atb)->write(f);
								else
									  static_cast<junction*>(atb)->write(f);
								break;
							case 0xa: static_cast<texture*>(atb)->write(f);            break;
							case 0xb: static_cast<facade*>(atb)->write(f);             break;
							case 0xc: static_cast<roof_triangle_fan*>(atb)->write(f);  break;
						}
			*/
			atb->write(f);

			++j;
		}

		//	tmp.seekg(0, ios::end);
		//	n_attributesize = tmp.tellg();

		n_attributesize = (f.tellp() - seek - sizeof(perimeter_pt) * n_perimeters) / 2 - 2;
		f.seekp(seek);
		f.write((char*)&n_attributesize, 4);
		f.seekp(0, ios_base::end);

		/*	if (n_attributesize > 1)
			{
				tmp.seekg(0, ios::beg);
				f << tmp.rdbuf();
			//	f.write((char*) tmp.str().c_str(), 1);
			}*/

			/*		while (j < n_attributes)
					{
						unsigned short id = 0;
						unsigned char type = 0, subtype = 0;
						bool last = false;

						attribute *atb = block->_attributes[j];

					//	ATLTRACE("Block %x: writing attribute %x ...\n", i, j);

						last	 = (atb->last || j == n_attributes-1) ^ def_last;
						type	 = atb->type;
						subtype	 = atb->subtype;

						def_last |= last;

						id = last << 7 | type << 3 | subtype;

						f.write((char*) &id, 2);

					//	unsigned short k;

						switch (type)
						{
							case 0x0: static_cast<road_strip*>(atb)->write(f);         break;
							case 0x1: static_cast<sidewalk_strip*>(atb)->write(f);     break;
							case 0x2: static_cast<rectangle_strip*>(atb)->write(f);    break;
							case 0x3: static_cast<sliver*>(atb)->write(f);             break;
							case 0x4: static_cast<crosswalk*>(atb)->write(f);          break;
							case 0x5: static_cast<road_triangle_fan*>(atb)->write(f);  break;
							case 0x6: static_cast<triangle_fan*>(atb)->write(f);       break;
							case 0x7: static_cast<facade_bound*>(atb)->write(f);       break;
							case 0x8: static_cast<divided_road_strip*>(atb)->write(f); break;
							case 0x9:
								if (subtype)
									  static_cast<tunnel*>(atb)->write(f);
								else
									  static_cast<junction*>(atb)->write(f);
								break;
							case 0xa: static_cast<texture*>(atb)->write(f);            break;
							case 0xb: static_cast<facade*>(atb)->write(f);             break;
							case 0xc: static_cast<roof_triangle_fan*>(atb)->write(f);  break;
						}

						++j;
					}
			*/
		ATLTRACE("Block %x: done!\n", i);
	}

	f.put(_unknown1);

	for (i = 0; i < n_size; ++i)
	{
		f.write((char*)&_blocks[i]->type, 1);
	}

	f.put(_unknown2);

	for (i = 0; i < n_size; ++i)
	{
		f.write((char*)&_blocks[i]->proprule, 1);
	}

	f.write((char*)&v_min, 12);
	f.write((char*)&v_max, 12);
	f.write((char*)&v_center, 12);
	f.write((char*)&f_radius, 4);

	n_size = num_blockpaths();
	f.write((char*)&n_size, 4);
	ATLTRACE("Number of blockpaths: 0x%x\n", n_size);

	for (i = 0; i < n_size; ++i)
	{
		blockpath* path = &_blockpaths[i];

		f.write((char*)&path->unknown4, 2);
		f.write((char*)&path->unknown5, 2);
		f.write((char*)&path->nFLanes, 1);
		f.write((char*)&path->nBLanes, 1);

		for (unsigned short j = 0; j < path->nFLanes + path->nBLanes; ++j)
			f.write((char*)&path->density[j], 4);

		f.write((char*)&path->unknown6, 2);

		unsigned char l;

		for (l = 0; l < 4; ++l)
			f.write((char*)&path->startCrossroads[l], 2);

		for (l = 0; l < 4; ++l)
			f.write((char*)&path->endCrossroads[l], 2);

		f.write((char*)&path->nRoadBlocks, 1);

		for (l = 0; l < path->nRoadBlocks; ++l)
		{
			f.write((char*)&path->roadBlocks[l], 2);
		}
	}

	if (!_junk.empty())
		f.write((char*)_junk[0], _junk.size());

	return error::ok;
}

error::code psdl::read_sdl(const char* filename, ProgressMonitor* monitor)
{
	unsigned long i, j;
	unsigned short k, n_size;
	string line, keyname;

	ifstream f(filename);

	if (!f.is_open()) return error::cant_open;

	//	ATLTRACE("\nReading SDL file: %s\n", filename);

	f.seekg(0, ios_base::end);
	int end = f.tellg();
	f.seekg(0, ios_base::beg);
	int beg = f.tellg();

	int fsize = end - beg;

	monitor->setMaximum(fsize);
	monitor->setCaption("Import SDL");
	monitor->setNote("Reading rooms");

	istringstream iss;

	unsigned long vertexoffset = num_vertices();
	unsigned long textureoffset = num_textures();
	unsigned long blockofset = num_blocks();

	while (getline(f, line))
	{
		iss.str(line);
		iss >> keyname;

		if (keyname == "v")
		{
			Vertex v;
			iss >> v.x >> v.y >> v.z;
			add_vertex(v);
		}
		else if (keyname == "td")
		{
			string td;
			iss >> td;
			add_texname(td);
		}
		else if (keyname == "rooms")
		{
			iss >> n_size;
			_blocks.reserve(n_size);
		}
		else if (keyname == "room")
		{
			i = 0;

			psdl::block* block = new psdl::block(&_vertices);
			add_block(block);

			attribute* atb = NULL;

			while (getline(f, line))
			{
				iss.str(line);
				iss >> keyname;

				if (keyname == "room")
				{
					++i;
					block = new psdl::block(&_vertices);
					add_block(block);

					monitor->setProgress(f.tellg());
				}
				else if (keyname == "type")
				{
					unsigned short type;
					iss >> type;
					block->type = type;
				}
				else if (keyname == "perimeter")
				{
					unsigned long n_perimeters;
					iss >> n_perimeters;

					unsigned short i_vertex, i_block;

					for (j = 0; j < n_perimeters; ++j)
					{
						iss >> i_vertex;
						iss >> i_block;

						block->add_perimeter_point(vertexoffset + i_vertex, blockofset + i_block);
					}
				}
				else if (keyname == "road")
				{
					atb = new road_strip(&_vertices);
					iss >> n_size;
					for (k = 0; k < n_size; ++k)
					{
						iss >> j;
						static_cast<road_strip*>(atb)->add_vertex(vertexoffset + j);
					}
					block->add_attribute(atb);
				}
				else if (keyname == "sidewalk")
				{
					atb = new sidewalk_strip(&_vertices);
					iss >> n_size;
					for (k = 0; k < n_size; ++k)
					{
						iss >> j;
						static_cast<sidewalk_strip*>(atb)->add_vertex(vertexoffset + j);
					}
					block->add_attribute(atb);
				}
				else if (keyname == "alley")
				{
					atb = new rectangle_strip(&_vertices);
					iss >> n_size;
					for (k = 0; k < n_size; ++k)
					{
						iss >> j;
						static_cast<rectangle_strip*>(atb)->add_vertex(vertexoffset + j);
					}
					block->add_attribute(atb);
				}
				else if (keyname == "crosswalk")
				{
					atb = new crosswalk(&_vertices);
					iss >> n_size; // Can only be 4
					for (k = 0; k < n_size; ++k)
					{
						iss >> j;
						static_cast<crosswalk*>(atb)->set_vertex(k, vertexoffset + j);
					}
					block->add_attribute(atb);
				}
				else if (keyname == "ofan2") // Road triangle fan
				{
					atb = new road_triangle_fan(&_vertices);
					iss >> n_size;
					for (k = 0; k < n_size; ++k)
					{
						iss >> j;
						static_cast<road_triangle_fan*>(atb)->add_vertex(vertexoffset + j);
					}
					block->add_attribute(atb);
				}
				else if (keyname == "ofan") // Plain triangle fan
				{
					atb = new triangle_fan(&_vertices);
					iss >> n_size;
					for (k = 0; k < n_size; ++k)
					{
						iss >> j;
						static_cast<triangle_fan*>(atb)->add_vertex(vertexoffset + j);
					}
					block->add_attribute(atb);
				}
				else if (keyname == "divroad")
				{
					atb = new divided_road_strip(&_vertices);
					iss >> n_size;

					unsigned short divider = 0;
					float value = 0;

					iss >> divider;
					iss >> value;

					static_cast<divided_road_strip*>(atb)->flags = divider;
					static_cast<divided_road_strip*>(atb)->height = divider == 2 ? value * 255.f : 0;

					for (k = 0; k < n_size - 2; ++k)
					{
						iss >> j;
						static_cast<divided_road_strip*>(atb)->add_vertex(vertexoffset + j);
					}
					block->add_attribute(atb);
				}
				else if (keyname == "tex")
				{
					atb = new texture();
					iss >> k;
					static_cast<texture*>(atb)->i_texture = textureoffset + k - 1; // 0 index will wrap to 0xffff
					block->add_attribute(atb);
				}

				iss.clear();
			}
		}

		iss.clear();

		monitor->setProgress(f.tellg());
	}

	//	monitor->done();
	return error::ok;
}
error::code psdl::write_blockdata(const char* filename)
{
	unsigned long i, j;

	ofstream f(filename);

	if (!f.is_open()) return error::cant_open;

	ATLTRACE("\nWriting Block Data: %s\n", filename);

	for (i = 0; i < num_blocks(); ++i)
	{
		block* block = _blocks[i];
		if (!(block->type & BIT_PLAIN)) continue;

		bool has_lot = false;
		string texname = "";
		for (j = 0; j < block->num_attributes(); ++j)
		{
			attribute* atb = block->_attributes[j];

			if (atb->type == ATB_TEXTURE)
			{
				texname = _textures[static_cast<texture*>(atb)->i_texture];
			}
			else if (atb->type == ATB_FACADE)
			{
				has_lot = true;
				break;
			}
		}

		if (!has_lot) continue;
		f << "block {" << "\n";

		f << "\tid: " << i + 1 << "\n";
		f << "\tperimeter: " << block->num_perimeters() << "\n";
		f << "\tverts: " << block->num_perimeters() << "\n";

		float ground = _vertices[block->_perimeter[0].vertex].y;

		for (j = 0; j < block->num_perimeters(); ++j)
		{
			Vertex& v = _vertices[block->_perimeter[j].vertex];
			f << "\tv " << v.x << " " << v.y << " " << v.z << "\n";

			if (v.y < ground) ground = v.y;
		}

		f << "\tlots: 1";

		//	streampos pos = f.tellp();

		f << "\n";

		f << "\tlot {" << "\n";
		f << "\t\tfloor \"nl_coitrock_f\"" << "\n";
		f << "\t\tsliver \"nl_coitrock_f\"" << "\n";
		f << "\t\tsliverVariant 0" << "\n";
		f << "\t\textra \"none\"" << "\n";
		f << "\t\trotation 0" << "\n";
		f << "\t\tscale 16" << "\n";
		f << "\t\theight " << ground + 30.0f << "\n";
		f << "\t\twalltile 0.00\n";
		f << "\t\tground 0.00\n";
		f << "\t\tpitch 0.00" << "\n";
		f << "\t\tverts: " << block->num_perimeters() << "\n";

		f << "\t\tperimeter ";
		for (j = 0; j < block->num_perimeters(); ++j)
		{
			f << j << " ";
		}
		f << "\n";

		f << "\t\tadjacent ";
		for (j = 0; j < block->num_perimeters(); ++j)
		{
			f << "-1 ";
		}
		f << "\n";

		f << "\t\twalls ";
		for (j = 0; j < block->num_perimeters(); ++j)
		{
			f << "\"" << texname << "\" ";
		}
		f << "\n";

		f << "\t\twallSlivers ";
		for (j = 0; j < block->num_perimeters(); ++j)
		{
			f << "\"sliver\" ";
		}
		f << "\n";

		f << "\t\twallSliverVariants ";
		for (j = 0; j < block->num_perimeters(); ++j)
		{
			f << "0 ";
		}
		f << "\n";

		f << "\t\twallBreak1 ";
		for (j = 0; j < block->num_perimeters(); ++j)
		{
			f << "10.00 ";
		}
		f << "\n";

		f << "\t\twallBreak2 ";
		for (j = 0; j < block->num_perimeters(); ++j)
		{
			f << "0.00 ";
		}
		f << "\n";

		f << "\t}" << "\n";

		f << "}" << "\n";
	}

	f << "end\n";

	f.flush();

	return error::ok;
}
error::code psdl::write_sdl(const char* filename, ProgressMonitor* monitor)
{
	unsigned long i, j;
	unsigned short k, n_size;

	ofstream f(filename);

	if (!f.is_open()) return error::cant_open;

	ATLTRACE("\nWriting SDL file: %s\n", filename);
	monitor->setMaximum(num_vertices() + num_textures() + num_blocks());
	unsigned int progress = num_vertices();

	monitor->setNote("Writing vertices");
	for (i = 0; i < num_vertices(); ++i)
	{
		f << "v " << _vertices[i].x << " " << _vertices[i].y << " " << _vertices[i].z << "\n";
		monitor->setProgress(i);
	}

	f << "\n";

	monitor->setNote("Writing textures");
	for (i = 0; i < num_textures(); ++i)
	{
		f << "td " << (_textures[i].empty() ? "rinter_x_l" : _textures[i].c_str()) << "\n";
		monitor->setProgress(++progress);
	}

	f << "\n";

	monitor->setNote("Writing rooms");
	f << "rooms " << num_blocks() + 1 << " 0 0\n\n";

	for (i = 0; i < num_blocks(); ++i)
	{
		block* block = _blocks[i];

		f << "room " << i + 1 << "\n";
		f << "id " << i + 1 << "\n";
		f << "type " << (unsigned short)block->type << "\n";
		f << "perimeter " << block->num_perimeters();

		for (j = 0; j < block->num_perimeters(); ++j)
		{
			f << "  " << block->_perimeter[j].vertex << " " << block->_perimeter[j].block;
		}

		f << "\n";

		int prev_tex_id = -1, tex_id = -1;

		for (j = 0; j < block->num_attributes(); ++j)
		{
			attribute* atb = block->_attributes[j];

			switch (atb->type)
			{
			case 0x0:
			case 0x1:
				if (static_cast<sidewalk_strip*>(atb)->is_endpiece()) continue; // Sidewalk end piece is unsupported

			case 0x2:
			case 0x4:
			case 0x5:
			case 0x6:
			case 0x8:
				if (tex_id != -1 && tex_id != prev_tex_id)
				{
					f << "tex " << tex_id << "\n";
					prev_tex_id = tex_id;
				}
			}

			/*
			switch (atb->type)
			{
				case 0x0:
					n_size = static_cast<road_strip*>(atb)->num_vertices();
					f << "road " << n_size << " ";
					for (k = 0; k < n_size; ++k)
					{
						f << " " << static_cast<road_strip*>(atb)->get_vertex(k) - _vertices.begin();
					}
					break;

				case 0x1:
					n_size = static_cast<sidewalk_strip*>(atb)->num_vertices();
					f << "sidewalk " << n_size << " ";
					for (k = 0; k < n_size; ++k)
					{
						f << " " << static_cast<sidewalk_strip*>(atb)->get_vertex(k) - _vertices.begin();
					}
					break;

				case 0x2:
					n_size = static_cast<rectangle_strip*>(atb)->num_vertices();
					f << "alley " << n_size << " ";
					for (k = 0; k < n_size; ++k)
					{
						f << " " << static_cast<rectangle_strip*>(atb)->get_vertex(k) - _vertices.begin();
					}
					break;

				case 0x4:
					f << "crosswalk 4 ";
					for (k = 0; k < 4; ++k)
					{
						f << " " << static_cast<crosswalk*>(atb)->get_vertex(k) - _vertices.begin();
					}
					break;

				case 0x5:
					n_size = static_cast<road_triangle_fan*>(atb)->num_vertices();
					f << "ofan2 " << n_size << " ";
					for (k = 0; k < n_size; ++k)
					{
						f << " " << static_cast<road_triangle_fan*>(atb)->get_vertex(k) - _vertices.begin();
					}
					break;

				case 0x6:
					n_size = static_cast<triangle_fan*>(atb)->num_vertices();
					f << "ofan " << n_size << " ";
					for (k = 0; k < n_size; ++k)
					{
						f << " " << static_cast<triangle_fan*>(atb)->get_vertex(k) - _vertices.begin();
					}
					break;

				case 0x8:
					n_size = static_cast<divided_road_strip*>(atb)->num_vertices();
					{
						unsigned short divider = static_cast<divided_road_strip*>(atb)->flags & 0x7;
					//	float value = divider == 2 ? (float) static_cast<divided_road_strip*>(atb)->value / 0xff : 0;
					//	unsigned short texture = static_cast<divided_road_strip*>(atb)->i_texture;
						float divider_height = static_cast<divided_road_strip*>(atb)->height / 255.f;

						f << "divroad " << n_size + 2 << " " << divider << " " << divider_height << " ";
					}

					for (k = 0; k < n_size; ++k)
					{
						f << " " << static_cast<divided_road_strip*>(atb)->get_vertex(k) - _vertices.begin();
					}
					break;

				case 0xa:
					tex_id = static_cast<texture*>(atb)->i_texture + 1;
					if (tex_id > 0xffff) tex_id = 0;

				default:
					continue;
			}

			f << "\n";
			*/
		}

		f << "\n";
		monitor->setProgress(++progress);
	}

	f.flush();

	return error::ok;
}
