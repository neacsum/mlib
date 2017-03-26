#pragma once

namespace UnitTest {

class Test;

class TestList
{
public:
    TestList();
    void Add (Test* test);

    Test* GetHead() const;

private:
    Test* head;
    Test* tail;
};


class ListAdder
{
public:
    ListAdder(TestList& list, Test* test);
};

}

