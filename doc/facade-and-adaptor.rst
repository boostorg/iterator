+++++++++++++++++++++++++++++
 Iterator Facade and Adaptor
+++++++++++++++++++++++++++++

:Author: David Abrahams, Jeremy Siek, Thomas Witt
:Contact: dave@boost-consulting.com, jsiek@osl.iu.edu, witt@ive.uni-hannover.de
:organization: `Boost Consulting`_, Indiana University `Open Systems Lab`_, University of Hanover `Institute for Transport Railway Operation and Construction`_
:date: $Date$

:copyright: Copyright Dave Abrahams, Jeremy Siek, and Thomas Witt 2003. All rights reserved

.. _`Boost Consulting`: http://www.boost-consulting.com
.. _`Open Systems Lab`: http://www.osl.iu.edu
.. _`Institute for Transport Railway Operation and Construction`: http://www.ive.uni-hannover.de

:abstract: We propose two class templates that help programmers
           build standard-conforming iterators and build
           iterators that adapt of other iterators.

.. contents:: Table of Contents

============
 Motivation
============

Iterators play an important role in modern C++ programming. The
iterator is the central abstraction of the algorithms of the Standard
Library, allowing algorithms to be re-used in in a wide variety of
contexts.

Iterators
=========

The power of iterators derives from several key features:

- Iterators form a rich family of concepts [#concept]_ whose
  functionality varies along several axes: movement, dereferencing,
  and associated type exposure.

- The existing iterator concepts of the C++ standard form a refinement
  hierarchy which allows the same basic interface elements to
  implement diverse functionality.

- Because built-in pointer types model the RandomAccessIterator
  concept, iterators can be both efficient and convenient to use.

The C++ Standard Library contains a wide variety of useful
iterators. Every one of the standard containers comes with constant
and mutable iterators[#mutable]_, and also reverse versions of those
same iterators which traverse the container in the opposite direction.
The Standard also supplies ``istream_iterator`` and
``ostream_iterator`` for reading from and writing to streams,
``insert_iterator``, ``front_insert_iterator`` and
``back_insert_iterator`` for inserting elements into containers, and
``raw_storage_iterator`` for initializing raw memory [7].

Despite the many iterators supplied by the Standard Library, many
obvious iterators are missing, and creating new iterator types is
still a common task for C++ programmers.  The literature documents
several of these, for example line_iterator [3] Constant_iterator
[9]. The iterator abstraction is so powerful, however, that we expect
programmers will always need to invent new iterator types.


Adaptors
========

Because iterators combine traversal, indirection, and associated type
exposure, it is common to want to adapt one iterator to form a new
one. This strategy allows one to reuse some of original iterator's
axes of variation while redefining others. For example, the Standard
provides reverse_iterator, which adapts any BidirectionalIterator by
inverting its direction of traversal.  As with plain iterators,
iterator adaptors defined outside the Standard have become commonplace
in the literature:


* Checked iter[13] adds bounds-checking to an existing iterator.

* The iterators of the View Template Library[14], which adapts
  containers, are themselves adaptors over the underlying iterators.


* smart iterators [5] adapt an iterator's dereferencing behavior by
  applying a function object to the object being referenced and
  returning the result.


* Custom iterators [4], in which a variety of adaptor types are enumerated.


* compound iterators [1], which access a slice out of a container of containers.


* Several iterator adaptors from the MTL [12]. The MTL contains a
  strided iterator, where each call to ``operator++()`` moves the
  iterator ahead by some constant factor, and a scaled iterator, which
  multiplies the dereferenced value by some constant.


.. [#concept] We use the term concept to mean a set of requirements
   that a type must satisfy to be used with a particular template
   parameter.

.. [#mutable] The term mutable iterator refers to iterators over objects that
   can be changed by assigning to the dereferenced iterator, while
   constant iterator refers to iterators over objects that cannot be
   modified.

To automate the repetitive work of constructing iterators, we propose
``iterator_facade``, an iterator base class template which provides
the rich interface of standard iterators and delegates its
implementation to member functions of the derived class.  We also
propose ``iterator_adaptor``, a base class generator designed
specifically for creating iterator adaptors.  Because iterators
usually have many of the features of their underlying iterator type,
the default features of ``iterator_adaptor`` are those of its base
[#base]_. The user can selectively replace these features in a
derived iterator class.

.. [#base] The term "Base" is not meant to imply the use of
   inheritance. We have followed the lead of the standard library,
   which provides a base() function to access the underlying iterator
   object of a reverse - iterator adaptor.

Core Elements of the Iterator Concept
=====================================

The first step in designing such a generalized model of the iterator concept is to identify
the core elements of its interface. We have identified the following core behaviors for
iterators:


* dereferencing
* incrementing
* decrementing
* equality comparison
* random-access motion
* distance measurement

In addition to the behaviors listed above, the core interface elements
include the associated types exposed through iterator traits: value
type, reference, pointer, and iterator category. The library supports
two ways of specifying these: as traditional template parameters and
also as named template parameters (described below), and uses a system
of smart defaults which in most cases reduces the number of these
types that must be specified.


From Building Models to Building Adaptors
=========================================

A generalized iterator generator is useful (helping to create new iterator types from
scratch), but a generalized iterator adaptor is even more useful. An adaptor generator
allows one to build whole families of iterator instances based on existing iterators.

In the Boost Iterator Adaptor Library, the iterator adaptor class template plays
the roles of both iterator generator and iterator adaptor generator. The behaviors of
iterator adaptor instances are supplied through a policies class [2] which allows
users to specialize adaptation. Users go beyond generating new iterator types to easily
generating new iterator adaptor families.

The library contains several examples of specialized adaptors which were quickly
implemented using iterator adaptor:


* Indirect Iterator Adaptor, which iterates over iterators, pointers, or smart pointers
  and applies an extra level of dereferencing.


* Reverse Iterator Adaptor, which inverts the direction of a Base iterator's motion,
  while allowing adapted constant and mutable iterators to interact in the expected
  ways. We will discuss this further in Section 5.2.1.

* Transform Iterator Adaptor, which applies a user-defined function object to the
  underlying values when dereferenced. We will show how this adaptor is implemented
  in Section 3.1.


* Projection Iterator Adaptor, which is similar to Transform Iterator Adaptor except
  that when dereferenced it returns by-reference instead of by-value.


* Filter Iterator Adaptor, which provides a view of an iterator range in which some
  elements of the underlying range are skipped.


* Counting Iterator Adaptor, which adapts any incrementable
  type (e.g. integers, iterators) so that incrementing/decrementing
  the adapted iterator and dereferencing it produces successive values
  of the Base type.


* Function Output Iterator Adaptor, which makes it easier to create custom output
  iterators.

Based on the examples in the library, users have generated many new adaptors,
among them a permutation adaptor which applies some permutation to a RandomAccessIterator,
and a strided adaptor, which adapts a RandomAccessIterator by multiplying
its unit of motion by a constant factor. In addition, the Boost Graph Library
(BGL) uses iterator adaptors to adapt other graph libraries, such as
LEDA [10] and Stanford GraphBase [8], to the BGL interface (which
requires C++ Standard compliant iterators).


The Boost iterator adaptor Class Template
=========================================

The iterator adaptor class template simplifies the creation of iterators by automating
the implementation of redundant operators and delegating functions and by taking
care of the complex details of iterator implementation.

The central design feature of iterator adaptor is parameterization by
a policies class. The policies class is the primary communication
mechanism between the iterator implementer and the iterator adaptor;
it specifies how the new iterator type behaves. Unlike the policy
classes in [2], we group several policies into a single class as this
proved more convenient for iterator implementation.

========================
 Impact on the Standard
========================

xxxx

========
 Design
========

xxx

===============
 Proposed Text
===============



::

  struct not_specified { };

  template <
      class Derived
    , class Value      = not_specified
    , class Category   = not_specified
    , class Reference  = not_specified
    , class Pointer    = not_specified
    , class Difference = not_specified
  >
  class iterator_facade;

  template <
      class Derived
    , class Base
    , class Value      = not_specified
    , class Category   = not_specified
    , class Reference  = not_specified
    , class Pointer    = not_specified
    , class Difference = not_specified
  >
  class iterator_adaptor;
  
  
  


``iterator_facade``
===================

::

  template <
      class Derived
    , class Value      = not_specified
    , class Category   = not_specified
    , class Reference  = not_specified
    , class Pointer    = not_specified
    , class Difference = not_specified
  >
  class iterator_facade {
  public:
      typedef ... value_type;
      typedef ... reference;
      typedef ... difference_type;
      typedef ... pointer;
      typedef ... iterator_category;

      reference operator*() const;
      <see details> operator->() const;
      <see details> operator[](difference_type n) const;
      Derived& operator++();
      Derived operator++(int);
      Derived& operator--();
      Derived operator--(int);
      Derived& operator+=(difference_type n);
      Derived& operator-=(difference_type n);
      Derived operator-(difference_type x) const;
  };

  // Comparison operators
  template <class Dr1, class V1, class C1, class R1, class P1, class D1,
            class Dr2, class V2, class C2, class R2, class P2, class D2>
  typename enable_if_interoperable<Dr1, Dr2, bool>::type
  operator ==(iterator_facade<Dr1, V1, C1, R1, P1, D1> const& lhs,
              iterator_facade<Dr2, V2, C2, R2, P2, D2> const& rhs);

  template <class Dr1, class V1, class C1, class R1, class P1, class D1,
            class Dr2, class V2, class C2, class R2, class P2, class D2>
  typename enable_if_interoperable<Dr1, Dr2, bool>::type
  operator !=(iterator_facade<Dr1, V1, C1, R1, P1, D1> const& lhs,
              iterator_facade<Dr2, V2, C2, R2, P2, D2> const& rhs);

  template <class Dr1, class V1, class C1, class R1, class P1, class D1,
            class Dr2, class V2, class C2, class R2, class P2, class D2>
  typename enable_if_interoperable<Dr1, Dr2, bool>::type
  operator <(iterator_facade<Dr1, V1, C1, R1, P1, D1> const& lhs,
             iterator_facade<Dr2, V2, C2, R2, P2, D2> const& rhs);

  template <class Dr1, class V1, class C1, class R1, class P1, class D1,
            class Dr2, class V2, class C2, class R2, class P2, class D2>
  typename enable_if_interoperable<Dr1, Dr2, bool>::type
  operator <=(iterator_facade<Dr1, V1, C1, R1, P1, D1> const& lhs,
              iterator_facade<Dr2, V2, C2, R2, P2, D2> const& rhs);

  template <class Dr1, class V1, class C1, class R1, class P1, class D1,
            class Dr2, class V2, class C2, class R2, class P2, class D2>
  typename enable_if_interoperable<Dr1, Dr2, bool>::type
  operator >(iterator_facade<Dr1, V1, C1, R1, P1, D1> const& lhs,
             iterator_facade<Dr2, V2, C2, R2, P2, D2> const& rhs);

  template <class Dr1, class V1, class C1, class R1, class P1, class D1,
            class Dr2, class V2, class C2, class R2, class P2, class D2>
  typename enable_if_interoperable<Dr1, Dr2, bool>::type
  operator >=(iterator_facade<Dr1, V1, C1, R1, P1, D1> const& lhs,
              iterator_facade<Dr2, V2, C2, R2, P2, D2> const& rhs);

  template <class Dr1, class V1, class C1, class R1, class P1, class D1,
            class Dr2, class V2, class C2, class R2, class P2, class D2>
  typename enable_if_interoperable<Dr1, Dr2, bool>::type
  operator >=(iterator_facade<Dr1, V1, C1, R1, P1, D1> const& lhs,
              iterator_facade<Dr2, V2, C2, R2, P2, D2> const& rhs);

  // Iterator difference
  template <class Dr1, class V1, class C1, class R1, class P1, class D1,
            class Dr2, class V2, class C2, class R2, class P2, class D2>
  typename enable_if_interoperable<Dr1, Dr2, bool>::type
  operator -(iterator_facade<Dr1, V1, C1, R1, P1, D1> const& lhs,
             iterator_facade<Dr2, V2, C2, R2, P2, D2> const& rhs);

  // Iterator addition
  template <class Derived, class V, class C, class R, class P, class D>
  Derived operator+ (iterator_facade<Derived, V, C, R, P, D> const&,
                     typename Derived::difference_type n)



``iterator_adaptor``
====================

::
  
  template <
      class Derived
    , class Base
    , class Value      = not_specified
    , class Category   = not_specified
    , class Reference  = not_specified
    , class Pointer    = not_specified
    , class Difference = not_specified
  >
  class iterator_adaptor : public iterator_facade<Derived, /*impl detail ...*/> {
  public:
      iterator_adaptor() {}
      explicit iterator_adaptor(Base iter);
      Base base() const;
  };
