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

:abstract: We propose a set of class templates that help programmers
           build standard-conforming iterators and to build iterators
           that adapt other iterators.

.. contents:: Table of Contents

============
 Motivation
============

Iterators play an important role in modern C++ programming. The
iterator is the central abstraction of the algorithms of the Standard
Library, allowing algorithms to be re-used in in a wide variety of
contexts.  The C++ Standard Library contains a wide variety of useful
iterators. Every one of the standard containers comes with constant
and mutable iterators [#mutable]_, and also reverse versions of those
same iterators which traverse the container in the opposite direction.
The Standard also supplies ``istream_iterator`` and
``ostream_iterator`` for reading from and writing to streams,
``insert_iterator``, ``front_insert_iterator`` and
``back_insert_iterator`` for inserting elements into containers, and
``raw_storage_iterator`` for initializing raw memory [7].

Despite the many iterators supplied by the Standard Library, obvious
and useful iterators are missing, and creating new iterator types is
still a common task for C++ programmers.  The literature documents
several of these, for example line_iterator [3] and Constant_iterator
[9].  The iterator abstraction is so powerful that we expect
programmers will always need to invent new iterator types.

Although it is easy to create iterators that *almost* conform to the
standard, the iterator requirements contain subtleties which can make
creating an iterator which *actually* conforms quite difficult.
Further, the iterator interface is rich, containing many operators
that are technically redundant and tedious to implement.  To automate
the repetitive work of constructing iterators, we propose
``iterator_facade``, an iterator base class template which provides
the rich interface of standard iterators and delegates its
implementation to member functions of the derived class.  In addition
to reducing the amount of code necessary to create an iterator, the
``iterator_facade`` also provides compile-time error detection.
Iterator implementation mistakes that often go unnoticed are turned
into compile-time errors because the derived class implementation must
match the expectations of the ``iterator_facade``.

A common pattern of iterator construction is the adaptation of one
iterator to form a new one.  The functionality of an iterator is
composed of four orthogonal aspects: traversal, indirection, equality
comparison and distance measurement.  Adapting an old iterator to
create a new one often saves work because one can reuse one aspect of
functionality while redefining the other.  For example, the Standard
provides ``reverse_iterator``, which adapts any Bidirectional Iterator
by inverting its direction of traversal.  As with plain iterators,
iterator adaptors defined outside the Standard have become commonplace
in the literature:

* Checked iter[13] adds bounds-checking to an existing iterator.

* The iterators of the View Template Library[14], which adapts
  containers, are themselves adaptors over the underlying iterators.

* Smart iterators [5] adapt an iterator's dereferencing behavior by
  applying a function object to the object being referenced and
  returning the result.

* Custom iterators [4], in which a variety of adaptor types are enumerated.

* Compound iterators [1], which access a slice out of a container of containers.

* Several iterator adaptors from the MTL [12].  The MTL contains a
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

To fulfill the need for constructing adaptors, we propose the
``iterator_adaptor`` class template.  Instantiations of
``iterator_adaptor`` serve as a base classes for new iterators,
providing the default behaviour of forwarding all operations to the
underlying iterator.  The user can selectively replace these features
in the derived iterator class.  This proposal also includes a number
of more specialized adaptors, such as the ``transform_iterator`` that
applies some user-specified function during the dereference of the
iterator.

========================
 Impact on the Standard
========================

This proposal is purely an addition to the C++ standard library.
However, note that this proposal relies on the proposal for New
Iterator Concepts.

========
 Design
========

Interoperability
================

The question of iterator interoperability is poorly adressed in the current standard.
There are currently two defect reports that are concerned with interoperability
issues.

Issue `179`_ concerns the fact that mutable container iterator types
are only required to be convertible the corresponding constant
iterator types, but objects of these types are not required to
interoperate in comparison or subtraction expressions.  This situation
is tedious in practice and out of line with the way built in types
work.  This proposal implements the proposed resolution to issue
`179`_, as most standard library implementations do nowadays. In other
words, if an iterator type A has an implicit or user defined
conversion to an iterator type B, the iterator types are interoperable
and the usual set of operators are available.

Issue `280`_ concerns the current lack of interoperability between
reverse iterator types. The proposed new reverse_iterator template
fixes the issues raised in 280. It provides the desired
interoperability without introducing unwanted overloads.

.. _`179`: http://anubis.dkuug.dk/jtc1/sc22/wg21/docs/lwg-defects.html#179
.. _`280`: http://anubis.dkuug.dk/jtc1/sc22/wg21/docs/lwg-active.html#280


Iterator Facade
===============

While the iterator interface is rich, there is a core subset of the
interface that is necessary for all the functionality.  We have
identified the following core behaviors for iterators:

* dereferencing
* incrementing
* decrementing
* equality comparison
* random-access motion
* distance measurement

In addition to the behaviors listed above, the core interface elements
include the associated types exposed through iterator traits:
``value_type``, ``reference``, ``pointer``, and ``iterator_category``.

Iterator facade uses the Curiously Recurring Template Pattern (CRTP)
[Cop95]_ so that the user can specifiy the behaviour of
``iterator_facade`` in a derived class.  Former designs used policy
objects to specifiy the behaviour.  The proposal does not use policy
objects for several reasons: 

  1. the creation and eventual copying of the policy object may create
     overhead that can be avoided with the current approach.

  2. The policy object approach does not allow for custom constructors
     on the created iterator types, an essential feature if
     ``iterator_facade`` should be used in other library
     implementations.

  3. Without the use of CRTP, the standard requirement that an
     iterator's ``operator++`` returns the iterator type itself means
     that all iterators generated by ``iterator_facade`` would be
     instantiations of ``iterator_facade``.  Cumbersome type generator
     metafunctions would be needed to build new parameterized
     iterators, and a separate ``iterator_adaptor`` layer would be
     impossible.

The user of ``iterator_facade`` derives his iterator class from an
instantiation of ``iterator_facade`` and defines member functions
implementing the core behaviors.  The following table describes
expressions which are required to be valid depending on the category
of the derived iterator type.

In the table below, ``X`` is the derived iterator type, ``a`` is an
object of type ``X``, ``b`` and ``c`` are objects of type ``const X``,
``n`` is an object of ``X::difference_type``, ``y`` is a constant
object of a single pass iterator type interoperable with X, and ``z``
is a constant object of a random access traversal iterator type
interoperable with X.

+----------------------------------------+----------------------------------------+-------------------------------------------------+-------------------------------------------+
| Expression                             | Return Type                            |    Assertion/Note/Precondition/Postcondition    | Required to implement Iterator Concept(s) |
|                                        |                                        |                                                 |                                           |
+========================================+========================================+=================================================+===========================================+
| ``c.dereference()``                    | ``X::reference``                       |                                                 | Readable Iterator, Writable Iterator      |
+----------------------------------------+----------------------------------------+-------------------------------------------------+-------------------------------------------+
| ``c.equal(b)``                         | convertible to bool                    |true iff ``b`` and ``c`` are equivalent.         | Single Pass Iterator                      |
+----------------------------------------+----------------------------------------+-------------------------------------------------+-------------------------------------------+
| ``c.equal(y)``                         | convertible to bool                    |true iff ``c`` and ``y`` refer to the same       | Single Pass Iterator                      |
|                                        |                                        |position.  Implements ``c == y`` and ``c != y``. |                                           |
+----------------------------------------+----------------------------------------+-------------------------------------------------+-------------------------------------------+
| ``a.advance(n)``                       | unused                                 |                                                 | Random Access Traversal Iterator          |
+----------------------------------------+----------------------------------------+-------------------------------------------------+-------------------------------------------+
| ``a.increment()``                      | unused                                 |                                                 | Incrementable Iterator                    |
+----------------------------------------+----------------------------------------+-------------------------------------------------+-------------------------------------------+
| ``a.decrement()``                      | unused                                 |                                                 | Bidirectional Traversal Iterator          |
+----------------------------------------+----------------------------------------+-------------------------------------------------+-------------------------------------------+
| ``c.distance_to(b)``                   | convertible to X::difference_type      | equivalent to ``distance(c, b)``                | Random Access Traversal Iterator          |
+----------------------------------------+----------------------------------------+-------------------------------------------------+-------------------------------------------+
| ``c.distance_to(z)``                   | convertible to X::difference_type      |equivalent to ``distance(c, z)``.  Implements ``c| Random Access Traversal Iterator          |
|                                        |                                        |- z``, ``c < z``, ``c <= z``, ``c > z``, and ``c |                                           |
|                                        |                                        |>= c``.                                          |                                           |
+----------------------------------------+----------------------------------------+-------------------------------------------------+-------------------------------------------+

.. Should we add a comment that a zero  overhead implementation of iterator_facade
   is possible with proper inlining?

Iterator Core Access
====================

``iterator_facade`` and the operator implementations need to be able
to access the core interface member functions in the derived class.
Making the core interface member funtions public would expose an
implementation detail to the user.  This proposal frees the public
interface of the derived iterator type from any implementation detail.

Preventing direct access to the core interface has two advantages.
First, there is no possibility for the user to accidently use a member
function of the iterator when a member of the value_type was intended.
This has been an issue with smart pointer implementations in the past.
The second and main advantage is that library implementers can freely
exchange a hand-rolled iterator implementation for one based on
``iterator_facade`` without fear of breaking code that was accessing
the public core interface directly.

In a naive implementation, keeping the derived class' core interface
private would require it to grant friendship to ``iterator_facade``
and each of the seven operators.  In order to reduce the burden of
limiting access, this proposal provides ``iterator_core_access``, a
class that acts as a gateway to the core interface in the derived
iterator class.  The author of the derived class only needs to grant
friendship to ``iterator_core_access`` to make his core interface
available to the library.

``iterator_core_access`` would be typically implemented as an empty
class containing only static member functions which invoke the
iterator core interface. There is, however, no need to standardize the
gateway protocol.

It is important to note that ``iterator_core_access`` does not open a
safety loophole, as every function in the core interface preserves the
invariants of the iterator.

Iterator Adaptor
================

The ``iterator_adaptor`` class template adapts some ``Base`` [#base]_
type to create a new iterator.  Instantiations of ``iterator_adaptor``
are derived from a corresponding instantiation of ``iterator_facade``
and implement the core behaviors in terms of the ``Base`` type. In
essence, ``iterator_adaptor`` merely forwards all operations to an
instance of the ``Base`` type, which it stores as a member.

.. [#base] The term "Base" here does not refer to a base class and is
   not meant to imply the use of derivation. We have followed the lead
   of the standard library, which provides a base() function to access
   the underlying iterator object of a ``reverse_iterator`` adaptor.

The user of ``iterator_adaptor`` creates a class derived from an
instantiation of ``iterator_adaptor`` and then selectively overrides
some of the core operations by implementing the (non-virtual) member
functions described in the table above.   

.. In addition, the derived
   class will typically need to define some constructors.

.. Jeremy, that last sentence is also true of iterator_facade.
   Perhaps we ought to cover the issue of constructors separately.

Specialized Adaptors
====================

This proposal also contains several examples of specialized adaptors
which were easily implemented using ``iterator_adaptor``:

* ``indirect_iterator``, which iterates over iterators, pointers,
  or smart pointers and applies an extra level of dereferencing.

* A new ``reverse_iterator``, which inverts the direction of a Base
  iterator's motion, while allowing adapted constant and mutable
  iterators to interact in the expected ways (unlike those in most
  implementations of C++98).

* ``transform_iterator``, which applies a user-defined function object
  to the underlying values when dereferenced.

* ``projection_iterator``, which is similar to ``transform_iterator``
  except that when dereferenced it returns a reference instead of
  a value.

* ``filter_iterator``, which provides a view of an iterator range in
  which some elements of the underlying range are skipped.

* ``counting_iterator``, which adapts any incrementable type
  (e.g. integers, iterators) so that incrementing/decrementing the
  adapted iterator and dereferencing it produces successive values of
  the Base type.

* ``function_output_iterator``, which makes it easier to create custom
  output iterators.

Based on examples in the Boost library, users have generated many new
adaptors, among them a permutation adaptor which applies some
permutation to a RandomAccessIterator, and a strided adaptor, which
adapts a RandomAccessIterator by multiplying its unit of motion by a
constant factor.  In addition, the Boost Graph Library (BGL) uses
iterator adaptors to adapt other graph libraries, such as LEDA [10]
and Stanford GraphBase [8], to the BGL interface (which requires C++
Standard compliant iterators).

===============
 Proposed Text
===============



::

  struct not_specified { };

  struct iterator_core_access { /* implementation detail */ };
  
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





.. [Cop95] [Coplien, 1995] Coplien, J., Curiously Recurring Template
   Patterns, C ++Report, February 1995, pp. 24-27.
