+++++++++++++++++++++++++++++
 Iterator Facade and Adaptor
+++++++++++++++++++++++++++++

:Author: David Abrahams, Jeremy Siek, Thomas Witt
:Contact: dave@boost-consulting.com, jsiek@osl.iu.edu, witt@ive.uni-hannover.de
:organization: `Boost Consulting`_, Indiana University `Open Systems Lab`_, University of Hanover `Institute for Transport Railway Operation and Construction`_
:date: $Date$
:Number: N1476=03-0059
:copyright: Copyright Dave Abrahams, Jeremy Siek, and Thomas Witt 2003. All rights reserved

.. _`Boost Consulting`: http://www.boost-consulting.com
.. _`Open Systems Lab`: http://www.osl.iu.edu
.. _`Institute for Transport Railway Operation and Construction`: http://www.ive.uni-hannover.de

:abstract: We propose a set of class templates that help programmers
           build standard-conforming iterators, both from scratch and
           by adapting other iterators.

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

Iterator Concepts
=================

This proposal is formulated in terms of the new ``iterator concepts`` 
as proposed in `n1477`_. This is due to the fact that especially 
adapted and user defined iterators suffer from the well known 
categorisation problems that are inherent to the current iterator
categories.

Though this proposal does not strictly depend on proposal `n1477`_,
as there is a direct mapping between new and old categories. The proposal
could be reformulated using this mapping.

Interoperability
================

The question of iterator interoperability is poorly adressed in the
current standard.  There are currently two defect reports that are
concerned with interoperability issues.

Issue `179`_ concerns the fact that mutable container iterator types
are only required to be convertible to the corresponding constant
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
``value_type``, ``reference``, ``pointer``, ``difference_type``, and
``iterator_category``.

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
of the derived iterator type.  These member functions are described
briefly below and in more detail in the `iterator facade requirements`_.

   +----------------------------------------+-------------------------------------------+
   | Expression                             | Effects                                   |
   +========================================+===========================================+
   | ``i.dereference()``                    | Access the value referred to              |
   +----------------------------------------+-------------------------------------------+
   | ``i.equal(j)``                         | Compare for equality with ``j``           |
   +----------------------------------------+-------------------------------------------+
   | ``i.increment()``                      | Advance by one position                   |
   +----------------------------------------+-------------------------------------------+
   | ``i.decrement()``                      | Retreat by one position                   |
   +----------------------------------------+-------------------------------------------+
   | ``i.advance(n)``                       | Advance by ``n`` positions                |
   +----------------------------------------+-------------------------------------------+
   | ``i.distance_to(j)``                   | Measure the distance to ``j``             |
   +----------------------------------------+-------------------------------------------+

.. Should we add a comment that a zero overhead implementation of iterator_facade
   is possible with proper inlining?

.. Would this be a good place to talk about constructors? -JGS

Iterator Core Access
====================

``iterator_facade`` and the operator implementations need to be able
to access the core member functions in the derived class.  Making the
core member funtions public would expose an implementation detail to
the user.  This proposal frees the public interface of the derived
iterator type from any implementation detail.

Preventing direct access to the core member functions has two
advantages.  First, there is no possibility for the user to accidently
use a member function of the iterator when a member of the value_type
was intended.  This has been an issue with smart pointer
implementations in the past.  The second and main advantage is that
library implementers can freely exchange a hand-rolled iterator
implementation for one based on ``iterator_facade`` without fear of
breaking code that was accessing the public core member functions
directly.

In a naive implementation, keeping the derived class' core member
functions private would require it to grant friendship to
``iterator_facade`` and each of the seven operators.  In order to
reduce the burden of limiting access, this proposal provides
``iterator_core_access``, a class that acts as a gateway to the core
member functions in the derived iterator class.  The author of the
derived class only needs to grant friendship to
``iterator_core_access`` to make his core member functions available
to the library.

``iterator_core_access`` would be typically implemented as an empty
class containing only static member functions which invoke the
iterator core member functions. There is, however, no need to
standardize the gateway protocol.

It is important to note that ``iterator_core_access`` does not open a
safety loophole, as every core member function preserves the
invariants of the iterator.

operator[]()
============

For operator[]() ``iterator_facade`` implements the semantics required
in the proposed resolution to issue `299`_ and adopted by proposal `n1477`_.
I.e. operator[]() is not required to return an lvalue.

.. _`299`: http://anubis.dkuug.dk/jtc1/sc22/wg21/docs/lwg-active.html#299

operator->()
============

For ``readable iterators`` the reference type is only required to 
be convertible to the value type. Though accessing members through
operator->() must still be possible. As a result a conformant
``readable iterator`` needs to return a proxy from operator->().

This proposal does not explicitly specify the return type for 
operator->() and operator[](). Instead it requires each ``iterator_facade``
instantiation to meet the requirements according to its ``iterator_category``.

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
functions described in the table above. The ``Base`` type
need not meet the full requirements for an iterator. It need
only support the operations that are not overriden by the
users derived class.


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
permutation to a random access iterator, and a strided adaptor, which
adapts a random access iterator by multiplying its unit of motion by a
constant factor.  In addition, the Boost Graph Library (BGL) uses
iterator adaptors to adapt other graph libraries, such as LEDA [10]
and Stanford GraphBase [8], to the BGL interface (which requires C++
Standard compliant iterators).

===============
 Proposed Text
===============

Header ``<iterator_helper>`` synopsis    [lib.iterator.helper.synopsis]
=======================================================================

.. How's that for a name for the header? -JGS
.. Also, below I changed "not_specified" to the user-centric "use_default" -JGS

.. Isn't use_default an implementation detail ? -thw

::

  struct use_default;

  struct iterator_core_access { /* implementation detail */ };
  
  template <
      class Derived
    , class Value      = use_default
    , class Category   = use_default
    , class Reference  = use_default
    , class Pointer    = use_default
    , class Difference = use_default
  >
  class iterator_facade;

  template <
      class Derived
    , class Base
    , class Value      = use_default
    , class Category   = use_default
    , class Reference  = use_default
    , class Pointer    = use_default
    , class Difference = use_default
  >
  class iterator_adaptor;
  
  template <
      class Iterator
    , class Value = use_default
    , class Category = use_default
    , class Reference = use_default
    , class Pointer = use_default
    , class Difference = use_default
  >
  class indirect_iterator;
  
  template <class Iterator>
  class reverse_iterator;

  template <class AdaptableUnaryFunction, class Iterator>
  class transform_iterator;

  template <class Predicate, class Iterator>
  class filter_iterator;

  template <
      class Incrementable, 
      class Category = use_default, 
      class Difference = use_default
  >
  class counting_iterator

  template <class UnaryFunction>
  class function_output_iterator;



Iterator facade [lib.iterator.facade]
=====================================


The iterator requirements define a rich interface, containing many
redundant operators, so that using iterators is convenient.  The
``iterator_facade`` class template makes it easier to create iterators
by implementing the rich interface of standard iterators in terms of a
few core functions.  The user of ``iterator_facade`` derives his
iterator class from an instantiation of ``iterator_facade`` and
defines member functions implementing the core behaviors.

.. Jeremy, I think this text is rather inappropriate for the
   standard.  This is not the place to discuss motivation, for
   example.  Compare with the standard text for vector:

     1 A vector is a kind of sequence that supports random access
     iterators. In addition, it supports (amortized) constant time
     insert and erase operations at the end; insert and erase in the
     middle take linear time. Storage management is handled
     automatically, though hints can be given to improve efficiency.

     2 A vector satisfies all of the requirements of a container and
     of a reversible container (given in two tables in 23.1) and of a
     sequence, including most of the optional sequence requirements
     (23.1.1). The exceptions are the push_front and pop_front member
     functions, which are not provided. Descriptions are pro- vided
     here only for operations on vector that are not described in one
     of these tables or for operations where there is additional
     semantic information.

   I suggest, instead:

     ``iterator_facade`` is a base class template which implements the
     interface of standard iterators in terms of a few core functions
     and associated types, to be supplied by a derived iterator class.

Template class ``iterator_facade``
----------------------------------

::

  template <
      class Derived
    , class Value      = use_default
    , class Category   = use_default
    , class Reference  = use_default
    , class Pointer    = use_default
    , class Difference = use_default
  >
  class iterator_facade {
  public:
      typedef ... value_type;
      typedef ... reference;
      typedef ... difference_type;
      typedef ... pointer;
      typedef ... iterator_category;

      reference operator*() const;
      /* see details */ operator->() const;
      /* see details */ operator[](difference_type n) const;
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



.. we need a new label here because the presence of markup in the
   title prevents an automatic link from being generated

.. _iterator facade requirements:

``iterator_facade`` requirements
--------------------------------

The ``Derived`` template parameter must be the class deriving from
``iterator_facade``.

.. We need to describe how the defaults work and what
   the typedefs come out to.  -JGS


The following table describes the requirements on the type deriving
from the ``iterator_facade``. The expressions listed in the table are
required to be valid depending on the category of the derived iterator
type.

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


.. We should explain more about how the
   functions in the interface of iterator_facade
   are there conditionally. -JGS


``iterator_facade`` operations
------------------------------


``reference operator*() const;``

:Returns: ``static_cast<Derived const*>(this)->dereference();``

*see details* ``operator->() const;``

:Requires: 
:Effects: 
:Postconditions: 
:Returns: ``static_cast<Derived const*>(this)->dereference();``
:Throws: 
:Complexity: 


*see details* ``operator[](difference_type n) const;``

:Requires: 
:Effects: 
:Postconditions: 
:Returns: ``static_cast<Derived const*>(this)->dereference();``
:Throws: 
:Complexity: 


``Derived& operator++();``

:Requires: 
:Effects: 
:Postconditions: 
:Returns: ``static_cast<Derived const*>(this)->dereference();``
:Throws: 
:Complexity: 

``Derived operator++(int);``

:Requires: 
:Effects: 
:Postconditions: 
:Returns: ``static_cast<Derived const*>(this)->dereference();``
:Throws: 
:Complexity: 

``Derived& operator--();``

:Requires: 
:Effects: 
:Postconditions: 
:Returns: ``static_cast<Derived const*>(this)->dereference();``
:Throws: 
:Complexity: 

``Derived operator--(int);``

:Requires: 
:Effects: 
:Postconditions: 
:Returns: ``static_cast<Derived const*>(this)->dereference();``
:Throws: 
:Complexity: 

``Derived& operator+=(difference_type n);``

:Requires: 
:Effects: 
:Postconditions: 
:Returns: ``static_cast<Derived const*>(this)->dereference();``
:Throws: 
:Complexity: 

``Derived& operator-=(difference_type n);``

:Requires: 
:Effects: 
:Postconditions: 
:Returns: ``static_cast<Derived const*>(this)->dereference();``
:Throws: 
:Complexity: 

``Derived operator-(difference_type x) const;``

:Requires: 
:Effects: 
:Postconditions: 
:Returns: ``static_cast<Derived const*>(this)->dereference();``
:Throws: 
:Complexity: 


Iterator adaptor [lib.iterator.adaptor]
=======================================

.. Jeremy, the same argument about appropriateness applies here as well.

A common pattern of iterator construction is the adaptation of one
iterator to form a new one.  The functionality of an iterator is
composed of four orthogonal aspects: traversal, indirection, equality
comparison, and distance measurement.  Adapting an old iterator to
create a new one often saves work because one can change a few aspects
of the functionality while retaining the rest.  For example, the
Standard provides ``reverse_iterator``, which adapts any Bidirectional
Iterator by inverting its direction of traversal.

The ``iterator_adaptor`` class template fulfills the need for
constructing adaptors.  Instantiations of ``iterator_adaptor`` serve
as a base classes for new iterators, providing the default behaviour
of forwarding all operations to the underlying iterator.  The user can
selectively replace these features in the derived iterator class.

The ``iterator_adaptor`` class template adapts a ``Base`` type to
create a new iterator ("base" here means the type being adapted).
The ``iterator_adaptor`` forwards all operations to an instance of the
``Base`` type, which it stores as a member.  The user of
``iterator_adaptor`` creates a class derived from an instantiation of
``iterator_adaptor`` and then selectively overrides some of the core
operations by implementing the (non-virtual) member functions
described in [lib.iterator.facade]. The ``Base`` type need not meet
the full requirements for an iterator. It need only support the
operations that are not overriden by the users derived class.

Template class ``iterator_adaptor``
-----------------------------------

::
  
  template <
      class Derived
    , class Base
    , class Value      = use_default
    , class Category   = use_default
    , class Reference  = use_default
    , class Pointer    = use_default
    , class Difference = use_default
  >
  class iterator_adaptor 
    : public iterator_facade<Derived, /*impl detail ...*/>
  {
      friend class iterator_core_access;
  public:
      iterator_adaptor() {}
      explicit iterator_adaptor(Base iter);
      Base base() const;
  };


``iterator_adaptor`` requirements
---------------------------------

Write me.

.. Make sure to mention that this words for both old and new
   style iterators. -JGS

.. I'm not sure we should say that in the standard text; let other
   people add non-normative  in editing if they feel the need
   ;-) -DWA

Specialized adaptors [lib.iterator.special.adaptors]
====================================================

.. The requirements for all of these need to be written *much* more
   formally -DWA


Indirect iterator
-----------------

The indirect iterator adapts an iterator by applying an *extra*
dereference inside of ``operator*()``. For example, this iterator
adaptor makes it possible to view a container of pointers
(e.g. ``list<foo*>``) as if it were a container of the pointed-to type
(e.g. ``list<foo>``) .


Template class ``indirect_iterator``
....................................

::

  template <
      class Iterator
    , class Value = use_default
    , class Category = use_default
    , class Reference = use_default
    , class Pointer = use_default
    , class Difference = use_default
  >
  class indirect_iterator
    : public iterator_adaptor</* see discussion */>
  {
      typedef iterator_adaptor</* see discussion */> super_t;
      friend class iterator_core_access;
   public:
      indirect_iterator() {}

      indirect_iterator(Iterator iter)
        : super_t(iter) {}

      template <
          class Iterator2, class Value2, class Category2
        , class Reference2, class Pointer2, class Difference2
      >
      indirect_iterator(
          indirect_iterator<
               Iterator2, Value2, Category2, Reference2, Pointer2, Difference2
          > const& y
        , typename enable_if_convertible<Iterator2, Iterator>::type* = 0
      )
        : super_t(y.base())
      {}

  private:    
      typename super_t::reference dereference() const
      {
          return **this->base();
      }
  };


``indirect_iterator`` requirements
..................................

The ``value_type`` of the ``Iterator`` template parameter should
itself be dereferenceable. The return type of the ``operator*`` for
the ``value_type`` must be the same type as the ``Reference`` template
parameter. The ``Value`` template parameter will be the ``value_type``
for the ``indirect_iterator``, unless ``Value`` is const. If ``Value``
is ``const X``, then ``value_type`` will be *non-* ``const X``.  The
default for ``Value`` is

::

  iterator_traits< iterator_traits<Iterator>::value_type >::value_type

If the default is used for ``Value``, then there must be a valid
specialization of ``iterator_traits`` for the value type of the base
iterator.

The ``Reference`` parameter will be the ``reference`` type of the
``indirect_iterator``. The default is ``Value&``.

The ``Pointer`` parameter will be the ``pointer`` type of the
``indirect_iterator``. The default is ``Value*``.

The ``Category`` parameter is the ``iterator_category`` type for the
``indirect_iterator``. The default is 
``iterator_traits<Iterator>::iterator_category``.

The indirect iterator will model whichever standard iterator concepts
are modeled by the base iterator. For example, if the base iterator is
a model of Random Access Traversal Iterator then so is the resulting
indirect iterator.

.. I don't believe the above statement is true anymore in light of the
   new categories.  I think it only applies to the traversal part of
   the concept.

Reverse iterator
----------------

The reverse iterator adaptor flips the direction of a base iterator's
motion. Invoking ``operator++()`` moves the base iterator backward and
invoking ``operator--()`` moves the base iterator forward.

Template class ``reverse_iterator``
...................................

::

  template <class Iterator>
  class reverse_iterator :
    public iterator_adaptor< reverse_iterator<Iterator>, Iterator >
  {
    typedef iterator_adaptor< reverse_iterator<Iterator>, Iterator > super_t;
    friend class iterator_core_access;
  public:
    reverse_iterator() {}

    explicit reverse_iterator(Iterator x) 
      : super_t(x) {}

    template<class OtherIterator>
    reverse_iterator(
        reverse_iterator<OtherIterator> const& r
      , typename enable_if_convertible<OtherIterator, Iterator>::type* = 0
    )
      : super_t(r.base())
    {}

  private: /* exposition */
    typename super_t::reference dereference() const { return *prior(this->base()); }
    
    void increment() { super_t::decrement(); }
    void decrement() { super_t::increment(); }

    void advance(typename super_t::difference_type n)
    {
      super_t::advance(-n);
    }

    template <class OtherIterator>
    typename super_t::difference_type
    distance_to(reverse_iterator<OtherIterator> const& y) const
    {
      return -super_t::distance_to(y);
    }
  };


``reverse_iterator`` requirements
.................................

The base iterator must be a model of Bidirectional Traversal
Iterator. The reverse iterator will model whichever standard iterator
concepts are modeled by the base iterator. For example, if the base
iterator is a model of Random Access Traversal Iterator then so is the
resulting reverse iterator.


Transform iterator
------------------

The transform iterator adapts an iterator by applying some function
object to the result of dereferencing the iterator. In other words,
the ``operator*`` of the transform iterator first dereferences the
base iterator, passes the result of this to the function object, and
then returns the result.


Template class ``transform_iterator``
.....................................

::

  template <class AdaptableUnaryFunction, class Iterator>
  class transform_iterator
    : public iterator_adaptor</* see discussion */>
  {
    typedef iterator_adaptor</* see discussion */> super_t;
    friend class iterator_core_access;
  public:
    transform_iterator() { }

    transform_iterator(Iterator const& x, AdaptableUnaryFunction f)
      : super_t(x), m_f(f) { }

    template<class OtherIterator>
    transform_iterator(
          transform_iterator<AdaptableUnaryFunction, OtherIterator> const& t
        , typename enable_if_convertible<OtherIterator, Iterator>::type* = 0
    )
      : super_t(t.base()), m_f(t.functor()) {}

    AdaptableUnaryFunction functor() const
      { return m_f; }

  private: /* exposition */
    typename super_t::value_type dereference() const
      { return m_f(super_t::dereference()); }

    AdaptableUnaryFunction m_f;
  };


``transform_iterator`` requirements
...................................

Write me. Use ``result_of``?


Filter iterator
---------------

The filter iterator adaptor creates a view of an iterator range in
which some elements of the range are skipped over. A predicate
function object controls which elements are skipped. When the
predicate is applied to an element, if it returns ``true`` then the
element is retained and if it returns ``false`` then the element is
skipped over.


Template class ``filter_iterator``
..................................

::

  template <class Predicate, class Iterator>
  class filter_iterator
      : public iterator_adaptor<
            filter_iterator<Predicate, Iterator>, Iterator
          , use_default
          , /* see details */
        >
  {
   public:
      filter_iterator() { }
      filter_iterator(Predicate f, Iterator x, Iterator end = Iterator());
      filter_iterator(Iterator x, Iterator end = Iterator());
      template<class OtherIterator>
      filter_iterator(
          filter_iterator<Predicate, OtherIterator> const& t
          , typename enable_if_convertible<OtherIterator, Iterator>::type* = 0
          );
      Predicate predicate() const;
      Iterator end() const;
  };



Counting iterator
-----------------


Template class ``counting_iterator``
....................................

::

  template <class Incrementable, class Category = not_specified, class Difference = not_specified>
  class counting_iterator
    : public iterator_adaptor</* see details */>
  {
      typedef iterator_adaptor</* see details */> super_t;
      friend class iterator_core_access;
   public:
      counting_iterator();
      counting_iterator(counting_iterator const& rhs);
      counting_iterator(Incrementable x);
  };


Function output iterator
------------------------

The function output iterator adaptor makes it easier to create custom
output iterators. The adaptor takes a unary function and creates a
model of Output Iterator. Each item assigned to the output iterator is
passed as an argument to the unary function.  The motivation for this
iterator is that creating a conforming output iterator is non-trivial,
particularly because the proper implementation usually requires a
proxy object.


Template class ``function_output_iterator``
...........................................

::

  template <class UnaryFunction>
  class function_output_iterator {
  public:
    typedef std::output_iterator_tag iterator_category;
    typedef void                value_type;
    typedef void                difference_type;
    typedef void                pointer;
    typedef void                reference;

    explicit function_output_iterator(const UnaryFunction& f = UnaryFunction())
      : m_f(f) {}

    struct output_proxy {
      output_proxy(UnaryFunction& f);
      template <class T> output_proxy& operator=(const T& value);
    };
    output_proxy operator*();
    function_output_iterator& operator++();
    function_output_iterator& operator++(int);
  };


.. [Cop95] [Coplien, 1995] Coplien, J., Curiously Recurring Template
   Patterns, C++ Report, February 1995, pp. 24-27.
