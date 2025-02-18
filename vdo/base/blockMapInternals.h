/*
 * Copyright (c) 2018 Red Hat, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA. 
 *
 * $Id: //eng/vdo-releases/aluminum/src/c++/vdo/base/blockMapInternals.h#10 $
 */

#ifndef BLOCK_MAP_INTERNALS_H
#define BLOCK_MAP_INTERNALS_H

#include "adminState.h"
#include "blockMapEntry.h"
#include "blockMapTree.h"
#include "completion.h"
#include "dirtyLists.h"
#include "header.h"
#include "intMap.h"
#include "ringNode.h"
#include "types.h"
#include "vdoPageCache.h"
#include "vioPool.h"

/**
 * The per-zone fields used by the block map tree.
 **/
struct blockMapTreeZone {
  /** The BlockMapZone which owns this tree zone */
  BlockMapZone        *mapZone;
  /** The lists of dirty tree pages */
  DirtyLists          *dirtyLists;
  /** The number of tree lookups in progress */
  VIOCount             activeLookups;
  /** The map of pages currently being loaded */
  IntMap              *loadingPages;
  /** The pool of VIOs for tree I/O */
  VIOPool             *vioPool;
  /** The tree page which has issued or will be issuing a flush */
  TreePage            *flusher;
  /** The queue of pages waiting for a flush so they can be written out */
  WaitQueue            flushWaiters;
  /** The generation after the most recent flush */
  uint8_t              generation;
  /** The oldest active generation */
  uint8_t              oldestGeneration;
  /** The counts of dirty pages in each generation */
  uint32_t             dirtyPageCounts[256];
};

/**
 * The per-zone fields of the block map.
 **/
struct blockMapZone {
  /** The number of the zone this is */
  ZoneCount         zoneNumber;
  /** The ID of this zone's logical thread */
  ThreadID          threadID;
  /** The BlockMap which owns this BlockMapZone */
  BlockMap         *blockMap;
  /** The ReadOnlyNotifier of the VDO */
  ReadOnlyNotifier *readOnlyNotifier;
  /** The page cache for this zone */
  VDOPageCache     *pageCache;
  /** The per-zone portion of the tree for this zone */
  BlockMapTreeZone  treeZone;
  /** The administrative state of the zone */
  AdminState        adminState;
};

struct blockMap {
  /** The manager for block map actions */
  ActionManager       *actionManager;
  /** The count of pages in the linear part of the block map */
  BlockCount           flatPageCount;
  /** The absolute PBN of the first root of the tree part of the block map */
  PhysicalBlockNumber  rootOrigin;
  /** The count of root pages of the tree part of the block map */
  BlockCount           rootCount;

  /** The era point we are currently distributing to the zones */
  SequenceNumber       currentEraPoint;
  /** The next era point, not yet distributed to any zone */
  SequenceNumber       pendingEraPoint;

  /** The number of entries in block map */
  BlockCount           entryCount;
  /** The VDO's nonce, for the pages */
  Nonce                nonce;
  /** The recovery journal for this map */
  RecoveryJournal     *journal;

  /** The trees for finding block map pages */
  Forest              *forest;
  /** The expanded trees awaiting growth */
  Forest              *nextForest;
  /** The number of entries after growth */
  BlockCount           nextEntryCount;

  /** The number of logical zones */
  ZoneCount            zoneCount;
  /** The per zone block map structure */
  BlockMapZone         zones[];
};

/**
 * Compute the number of pages required for a block map with the specified
 * parameters.
 *
 * @param entries   The number of block map entries
 *
 * @return The number of pages required
 **/
PageCount computeBlockMapPageCount(BlockCount entries);

/**
 * Compute the number of the block map page on which the entry for a given
 * logical block resides.
 *
 * @param lbn  The logical block number whose page is desired
 *
 * @return The number of the block map page containing the entry for
 *         the given logical block number
 **/
__attribute__((warn_unused_result))
static inline PageNumber computePageNumber(LogicalBlockNumber lbn)
{
  return (lbn / BLOCK_MAP_ENTRIES_PER_PAGE);
}

/**
 * Find the block map page slot in which the entry for a given logical
 * block resides.
 *
 * @param lbn  The logical block number whose slot
 *
 * @return The slot containing the entry for the given logical block number
 **/
__attribute__((warn_unused_result))
static inline SlotNumber computeSlot(LogicalBlockNumber lbn)
{
  return (lbn % BLOCK_MAP_ENTRIES_PER_PAGE);
}

#endif // BLOCK_MAP_INTERNALS_H
