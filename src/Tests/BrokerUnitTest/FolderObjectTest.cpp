// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include <gtest/gtest.h>

#include <dcclite/FolderObject.h>

TEST(FolderObject, ParentTest)
{
	using namespace dcclite;

	FolderObject folder(RName{ "root" });

	ASSERT_EQ(folder.GetParent(), nullptr);

	ASSERT_EQ(&folder.GetRoot(), &folder);

	auto sub1 = std::make_unique<FolderObject>(RName{ "sub1" });
	auto psub1 = sub1.get();

	folder.AddChild(std::move(sub1));	
	ASSERT_EQ(&psub1->GetRoot(), &folder);
}

