++++++++++++++++++++++
 New Iterator Concepts
++++++++++++++++++++++

:Author: David Abrahams, Jeremy Siek, Thomas Witt
:Contact: dave@boost-consulting.com, jsiek@osl.iu.edu, witt@ive.uni-hannover.de
:organization: `Boost Consulting`_, Indiana University `Open Systems Lab`_, University of Hanover `Institute for Transport Railway Operation and Construction`_
:date: $Date$

:copyright: Copyright Dave Abrahams, Jeremy Siek, and Thomas Witt 2003. All rights reserved

.. _`Boost Consulting`: http://www.boost-consulting.com
.. _`Open Systems Lab`: http://www.osl.iu.edu
.. _`Institute for Transport Railway Operation and Construction`: http://www.ive.uni-hannover.de

:abstract: We propose a new system of iterator concepts that treat
           access and positioning independently. This allows the
           concepts to more closely match the requirements
           of algorithms and provides better categorizations
           of iterators that are used in practice.
          
.. contents:: Table of Contents

============
 Motivation
============

The standard iterator categories and requirements are flawed because
they use a single hierarchy of concepts to address two orthogonal
issues: *iterator traversal* and *value access*. The current iterator
concept hierarchy is geared towards iterator traversal (hence the
category names), while requirements that address value access sneak in
at various places. The following table gives a summary of the current
value access requirements in the iterator categories.

+------------------------+-------------------------------------------------------------------------+
| Output Iterator        |  ``*i = a``                                                             |
+------------------------+-------------------------------------------------------------------------+
| Input Iterator         | ``*i`` is convertible to ``T``                                          |
+------------------------+-------------------------------------------------------------------------+
| Forward Iterator       | ``*i`` is ``T&`` (or ``const T&`` once                                  |
|                        | `issue 200`_ is resolved)                                               |
+------------------------+-------------------------------------------------------------------------+
| Random Access Iterator | ``i[n]`` is convertible to ``T`` (also ``i[n] = t`` is required for     |
|                        | mutable iterators once `issue 299`_ is resolved)                        |
+------------------------+-------------------------------------------------------------------------+

.. _issue 200: http://anubis.dkuug.dk/JTC1/SC22/WG21/docs/lwg-active.html#200
.. _issue 299: http://anubis.dkuug.dk/JTC1/SC22/WG21/docs/lwg-active.html#299


Because iterator traversal and value access are mixed together in a
single hierarchy, many useful iterators can not be appropriately
categorized. For example, ``vector<bool>::iterator`` is almost a
random access iterator, but the return type is not ``bool&`` (see
`issue 96`_ and Herb Sutter's paper J16/99-0008 = WG21
N1185). Therefore, its iterators only meet the requirements of input
iterator and output iterator. This is so nonintuitive that at least
one implementation erroneously assigns ``random_access_iterator_tag``
as its ``iterator_category``. Also, ``vector<bool>`` is not the only
example of useful iterators that do not return true references: there
is the often cited example of disk-based collections.

.. _issue 96: http://anubis.dkuug.dk/JTC1/SC22/WG21/docs/lwg-active.html#96

Another example is a counting iterator, an iterator the returns a
sequence of integers when incremented and dereferenced (see
counting_iterator_).  There are two ways to implement this iterator,
1) return a true reference from ``operator[]`` (a reference to an
integer data member of the counting iterator) or 2) return the
``value_type`` or a proxy from ``operator[]``. Option 1) runs into the
problems discussed in `issue 198`_, the reference will not be valid
after the iterator is destroyed. Option 2) is therefore a better
choice, but then we have a counting iterator that cannot be a random
access iterator.

.. _counting_iterator: http://www.boost.org/libs/utility/counting_iterator.htm
.. _issue 198: http://anubis.dkuug.dk/JTC1/SC22/WG21/docs/lwg-active.html#198

Yet another example is a transform iterator, an iterator adaptor that
applies a unary function object to the dereference value of the
wrapped iterator (see `transform_iterator`_).  For unary functions
such as ``times`` the return type of ``operator*`` clearly needs
to be the ``result_type`` of the function object, which is typically
not a reference. However, with the current iterator requirements, if
you wrap ``int*`` with a transform iterator, you do not get a random
access iterator as expected, but an input iterator.

.. _`transform_iterator`: http://www.boost.org/libs/utility/transform_iterator.htm

A fourth example is found in the vertex and edge iterators of the
`Boost Graph Library`_. These iterators return vertex and edge
descriptors, which are lightweight handles created on-the-fly. They
must be returned by-value. As a result, their current standard
iterator category is ``input_iterator_tag``, which means that,
strictly speaking, you could not use these iterators with algorithms
like ``min_element()``. As a temporary solution, the concept
`Multi-Pass Input Iterator`_ was introduced to describe the vertex and
edge descriptors, but as the design notes for concept suggest, a
better solution is needed.

.. _Boost Graph Library: http://www.boost.org/libs/graph/doc/table_of_contents.html
.. _Multi-Pass Input Iterator: http://www.boost.org/libs/utility/MultiPassInputIterator.html

In short, there are many useful iterators that do not fit into the
current standard iterator categories. As a result, the following bad
things happen:

- Iterators are often miss-categorized. 
- Algorithm requirements are more strict than necessary, because they can 
  not separate out the need for random-access from the need for a true reference 
  return type.


========================
 Impact on the Standard
========================

The new iterator concepts are backwards compatible with the old
iterator requirements. Iterators that satisfy the old requirements
also satisfy appropriate concepts in the new system, and iterators
modeling the new concepts will automatically satisfy the appropriate
old requirements.

The algorithms in the standard library benefit from the new iterator
concepts because the new concepts provide a more accurate way to
express their type requirements. The result is algorithms that are
usable in more situations and have fewer type requirements.

Forward Iterator -> Forward Traversal Iterator and Readable Iterator
  ``find_end, find_first_of, adjacent_find, search, search_n, rotate_copy, lower_bound, upper_bound, equal_range, binary_search, min_element, max_element``

Forward Iterator -> Readable Iterator and Writable Iterator
  ``iter_swap``

Forward Iterator -> Forward Traversal Iterator and Writable Iterator
  ``fill, generate``

Forward Iterator -> Forward Traversal Iterator and Swappable Iterator
  ``swap_ranges, rotate``

Forward Iterator -> Forward Traversal Iterator and Readable Iterator and Writable Iterator
  ``replace, replace_if, remove, remove_if, unique``

Bidirectional Iterator -> Bidirectional Traversal Iterator and Swappable Iterator
  ``reverse, partition``

Bidirectional Iterator -> Bidirectional Traversal Iterator and Readable Iterator, Bidirectional Iterator -> Bidirectional Traversal Iterator and Writable Iterator
  ``copy_backwards``

Bidirectional Iterator -> Bidirectional Traversal Iterator and Swappable Iterator and Readable Iterator
  ``next_permutation, prev_permutation``

Bidirectional Iterator -> Bidirectional Traversal Iterator and Readable Iterator and Writable Iterator
  ``stable_partition, inplace_merge``

Bidirectional Iterator -> Bidirectional Traversal Iterator and Readable Iterator
  ``reverse_copy``

Random Access Iterator -> Random Access Traversal Iterator and Swappable Iterator
  ``random_shuffle, sort, stable_sort, partial_sort, nth_element, push_heap, pop_heap
  make_heap, sort_heap``


========
 Design
========

The iterator requirements are be separated into two hierarchies. One
set of concepts handles the syntax and semantics of value access:

- Readable Iterator
- Writable Iterator
- Swappable Iterator
- Readable Lvalue Iterator
- Writable Lvalue Iterator

The refinement relationships among these iterator concepts are given
in the following diagram.

.. image:: access.png

The access concepts describe requirements related to ``operator*`` and
``operator->``, including the ``value_type``, ``reference``, and
``pointer`` associated types.

The other set of concepts handles traversal:

- Incrementable Iterator
- Single Pass Iterator
- Forward Traversal Iterator
- Bidirectional Traversal Iterator
- Random Access Traversal Iterator

The refinement relationships for the traversal concepts are in the
following diagram.

.. image:: traversal.png

In addition to the iterator movement operators, such as
``operator++``, the traversal concepts also include requirements on
position comparison such as ``operator==`` and ``operator<``.  The
reason for the fine grain slicing of the concepts into the
Incrementable and Single Pass is to provide concepts that are exact
matches with the original input and output iterator requirements.

The relationship between the new iterator concepts and the old are
given in the following diagram.

.. image:: oldeqnew.png

Like the old iterator requirements, we provide tags for purposes of
dispatching. There are two hierarchies of tags, one for the access
concepts and one for the traversal concepts. We provide an access
mechanism for mapping iterator types to these new tags. Our design
opts to reuse ``iterator_traits<Iter>::iterator_category`` as the
access mechanism. To enable this, a pair of access and traversal tags
are combined using the new `iterator_tag` class.

::

  template <class AccessTag, class TraversalTag>
  struct iterator_tag : <appropriate old category>
  {
    typedef AccessTag access;
    typedef TraversalTag traversal;
  };

The ``iterator_tag`` class template inherits the appropriate iterator
tag from the old requirements based on the new tags of the
iterator. The algorithm for determining the old tag from the new tags
picks the smallest old concept that includes all of the requirements
of the access and traversal concepts.

We also provide two helper classes that make it convenient to obtain
the access and traversal tags of an iterator. These helper classes
work both for iterators whose ``iterator_category`` is
``iterator_tag`` and also for iterators using the original iterator
categories.

::

  template <class Iterator> struct access_category { typedef ... type; };
  template <class Iterator> struct traversal_category { typedef ... type; };


The most difficult design decision concerned the ``operator[]``. The
direct approach for specifying ``operator[]`` would have a return type
of ``reference``; the same as ``operator*``. However, going in this
direction would mean that an iterator satisfying the old Random Access
Iterator requirements would not necessarily be a model of Readable or
Writable Lvalue Iterator. Instead we have chosen a design that matches
the resolution of `issue 299`_. So ``operator[]`` is only required to
return something convertible to the ``value_type`` (for a Readable
Iterator), and is required to support assignment ``i[n] = t`` (for a
Writable Iterator).


===============
 Proposed Text
===============

Addition to [lib.iterator.requirements]
=======================================

Iterator Value Access Concepts [lib.iterator.value.access]
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Readable Iterators [lib.readable.iterators]
-------------------------------------------

A class or built-in type ``X`` models the *Readable Iterator* concept
for the value type ``T`` if the following expressions are valid and
respect the stated semantics. ``U`` is the type of any specified
member of type ``T``.

 +------------------------------------------------------------------------------------------------------------------------------------------------------------+
 | Readable Iterator Requirements (in addition to CopyConstructible)                                                                                          |
 +--------------------------------------+---------------------------------------------------+-----------------------------------------------------------------+
 | Expression                           | Return Type                                       | Assertion/Note/Precondition/Postcondition                       |
 +======================================+===================================================+=================================================================+
 | ``iterator_traits<X>::value_type``   | ``T``                                             | Any non-reference, non-cv-qualified type                        |
 +--------------------------------------+---------------------------------------------------+-----------------------------------------------------------------+
 | ``iterator_traits<X>::reference``    | Convertible to ``iterator_traits<X>::value_type`` |                                                                 |
 +--------------------------------------+---------------------------------------------------+-----------------------------------------------------------------+
 | ``access_category<X>::type``         | Convertible to ``readable_iterator_tag``          |                                                                 |
 +--------------------------------------+---------------------------------------------------+-----------------------------------------------------------------+
 | ``*a``                               | ``iterator_traits<X>::reference``                 | pre: ``a`` is dereferenceable. If ``a == b`` then               |
 |                                      |                                                   | ``*a`` is equivalent to ``*b``                                  |
 +--------------------------------------+---------------------------------------------------+-----------------------------------------------------------------+
 | ``a->m``                             | ``U&``                                            | pre: ``(*a).m`` is well-defined.  Equivalent to ``(*a).m``      |
 +--------------------------------------+---------------------------------------------------+-----------------------------------------------------------------+


Writable Iterators [lib.writable.iterators]
-------------------------------------------

A class or built-in type ``X`` models the *Writable Iterator* concept
if the following expressions are valid and respect the stated
semantics.  A type ``T`` belongs to the *set of value types* of ``X``
if, for an object ``t`` of type ``T``, ``*a = t`` is valid.

 +------------------------------------------------------------------------------------------------------------------------------+
 | Writable Iterator Requirements (in addition to CopyConstructible)                                                            |
 +--------------------------------------+------------------------------------------+--------------------------------------------+
 | Expression                           | Return Type                              |  Assertion/Note/Precondition/Postcondition |
 +======================================+==========================================+============================================+
 | ``access_category<X>::type``         | Convertible to ``writable_iterator_tag`` |                                            |
 +--------------------------------------+------------------------------------------+--------------------------------------------+
 | ``*a = t``                           |                                          | pre: The type of ``t`` is in the set of    |
 |                                      |                                          | value types of ``X``                       |
 +--------------------------------------+------------------------------------------+--------------------------------------------+


Swappable Iterators [lib.swappable.iterators]
---------------------------------------------

A class or built-in type ``X`` models the *Swappable Iterator* concept
if the following expressions are valid and respect the stated
semantics.

 +------------------------------------------------------------------------------------------------+
 | Swappable Iterator Requirements (in addition to CopyConstructible)                             |
 +------------------------------------+-------------+---------------------------------------------+
 | Expression                         | Return Type |  Assertion/Note/Precondition/Postcondition  |
 +====================================+=============+=============================================+
 | ``iter_swap(a, b)``                | ``void``    |  post: the pointed to values are exchanged  |
 +------------------------------------+-------------+---------------------------------------------+

[*Note:* An iterator that is a model of the *Readable* and *Writable Iterator* concepts
  is also a model of *Swappable Iterator*.  *--end note*]


Readable Lvalue Iterators [lib.readable.lvalue.iterators]
---------------------------------------------------------

The *Readable Lvalue Iterator* concept adds the requirement that the
``reference`` type be a reference to the value type of the iterator.

 +--------------------------------------------------------------------------------------------------------------------------------------------------+
 | Readable Lvalue Iterator Requirements (in addition to Readable Iterator)                                                                         |
 +------------------------------------+-------------------------------------------------+-----------------------------------------------------------+
 | Expression                         | Return Type                                     |  Assertion/Note/Precondition/Postcondition                |
 +====================================+=================================================+===========================================================+
 | ``iterator_traits<X>::reference``  | ``T&``                                          | ``T`` is *cv* ``iterator_traits<X>::value_type`` where    |
 |                                    |                                                 |  *cv* is an optional cv-qualification                     |
 +------------------------------------+-------------------------------------------------+-----------------------------------------------------------+
 | ``access_category<X>::type``       | Convertible to ``readable_lvalue_iterator_tag`` |                                                           |
 +------------------------------------+-------------------------------------------------+-----------------------------------------------------------+


Writable Lvalue Iterators [lib.writable.lvalue.iterators]
---------------------------------------------------------

The *Writable Lvalue Iterator* concept adds the requirement that the
``reference`` type be a non-const reference to the value type of the
iterator.

 +------------------------------------------------------------------------------------------------------------------------------------------------------+
 | Writable Lvalue Iterator Requirements (in addition to Readable Lvalue Iterator)                                                                      |
 +--------------------------------------+--------------------------------------------------+------------------------------------------------------------+
 | Expression                           | Return Type                                      | Assertion/Note/Precondition/Postcondition                  |
 +======================================+==================================================+============================================================+
 | ``iterator_traits<X>::reference``    | ``iterator_traits<X>::value_type&``              |                                                            |
 +--------------------------------------+--------------------------------------------------+------------------------------------------------------------+
 | ``access_category<X>::type``         | Convertible to ``writable_lvalue_iterator_tag``  |                                                            |
 +--------------------------------------+--------------------------------------------------+------------------------------------------------------------+


Iterator Traversal Concepts [lib.iterator.traversal]
++++++++++++++++++++++++++++++++++++++++++++++++++++

Incrementable Iterators [lib.incrementable.iterators]
-----------------------------------------------------

A class or built-in type ``X`` models the *Incrementable Iterator*
concept if the following expressions are valid and respect the stated
semantics.


 +------------------------------------------------------------------------------------------------------------------------------------------------------+
 | Incrementable Iterator Requirements (in addition to Assignable, Copy Constructible)                                                                  |
 +--------------------------------------+--------------------------------------------------+------------------------------------------------------------+
 | Expression                           | Return Type                                      | Assertion/Note/Precondition/Postcondition                  |
 +======================================+==================================================+============================================================+
 | ``++r``                              | ``X&``                                           | ``&r == &++r``                                             |
 +--------------------------------------+--------------------------------------------------+------------------------------------------------------------+
 | ``r++``                              | convertible to ``const X&``                      | ``{ X tmp = r; ++r; return tmp; }``                        |
 +--------------------------------------+--------------------------------------------------+------------------------------------------------------------+
 | ``traversal_category<X>::type``      |                                                  | Convertible to ``incrementable_iterator_tag``              |
 +--------------------------------------+--------------------------------------------------+------------------------------------------------------------+


Single Pass Iterators [lib.single.pass.iterators]
-------------------------------------------------

A class or built-in type ``X`` models the *Single Pass Iterator*
concept if the following expressions are valid and respect the stated
semantics.

 +------------------------------------------------------------------------------------------------------------------------------------------------------+
 | Single Pass Iterator Requirements (in addition to Incrementable Iterator and Equality Comparable)                                                    |
 +----------------------------------+-------------------------+-----------------------------------------------------------------------------------------+
 | Expression                       | Return Type             | Assertion/Note/Precondition/Postcondition/Semantics                                     |
 +==================================+=========================+=========================================================================================+
 | ``++r``                          | ``X&``                  | pre: ``r`` is dereferenceable; post: ``r`` is dereferenceable or ``r`` is past-the-end  |
 +----------------------------------+-------------------------+-----------------------------------------------------------------------------------------+
 | ``a != b``                       | convertible to ``bool`` | ``!(a == b)``                                                                           |
 +----------------------------------+-------------------------+-----------------------------------------------------------------------------------------+
 | ``traversal_category<X>::type``  |                         | Convertible to ``single_pass_iterator_tag``                                             |
 +----------------------------------+-------------------------+-----------------------------------------------------------------------------------------+


Forward Traversal Iterators [lib.forward.traversal.iterators]
-------------------------------------------------------------

A class or built-in type ``X`` models the *Forward Traversal Iterator*
concept if the following expressions are valid and respect the stated
semantics.

 +----------------------------------------------------------------------------------------------------------------------------------+
 | Forward Traversal Iterator Requirements (in addition to Single Pass Iterator)                                                    |
 +------------------------------------------+--------------+------------------------------------------------------------------------+
 | Expression                               | Return Type  |   Assertion/Note/Precondition/Postcondition/Semantics                  |
 +==========================================+==============+========================================================================+
 | ``++r``                                  | ``X&``       |   ``r == s`` and ``r`` is dereferenceable implies ``++r == ++s.``      |
 +------------------------------------------+--------------+------------------------------------------------------------------------+
 | ``iterator_traits<X>::difference_type``  |              |   A signed integral type representing the distance between iterators   |
 +------------------------------------------+--------------+------------------------------------------------------------------------+
 | ``traversal_category<X>::type``          |              |   Convertible to ``forward_traversal_iterator_tag``                    |
 +------------------------------------------+--------------+------------------------------------------------------------------------+


Bidirectional Traversal Iterators [lib.bidirectional.traversal.iterators]
-------------------------------------------------------------------------

A class or built-in type ``X`` models the *Bidirectional Traversal
Iterator* concept if the following expressions are valid and respect
the stated semantics.

   +-------------------------------------------------------------------------------------------------------------+
   |Bidirectional Traversal Iterator Requirements (in addition to Forward Traversal Iterator)                    |
   +-----------------------------------------+-------------+-----------------------------------------------------+
   | Expression                              | Return Type | Assertion/Note/Precondition/Postcondition/Semantics |
   +=========================================+=============+=====================================================+
   | ``--r``                                 | ``X&``      |pre: there exists ``s`` such that ``r == ++s``.      |
   |                                         |             |post: ``s`` is dereferenceable. ``--(++r) == r``.    |
   |                                         |             |``--r == --s`` implies ``r == s``. ``&r == &--r``.   |
   +-----------------------------------------+-------------+-----------------------------------------------------+
   |``r--``                                  |convertible  |``{ X tmp = r; --r; return tmp; }``                  |
   |                                         |to const     |                                                     |
   |                                         |``X&``       |                                                     |
   +-----------------------------------------+-------------+-----------------------------------------------------+
   | ``traversal_category<X>::type``         |             | Convertible to                                      |
   |                                         |             | ``bidirectional_traversal_iterator_tag``            |
   |                                         |             |                                                     |
   +-----------------------------------------+-------------+-----------------------------------------------------+


Random Access Traversal Iterators [lib.random.access.traversal.iterators]
-------------------------------------------------------------------------

A class or built-in type ``X`` models the *Random Access Traversal
Iterator* concept if the following expressions are valid and respect
the stated semantics.

   +------------------------------------------------------------------------------------------------------------------------------------------------+
   | Random Access Traversal Iterator Requirements (in addition to Bidirectional Traversal Iterator)                                                |
   +--------------------------------+---------------------------------------+------------------------------+----------------------------------------+
   | Expression                     | Return Type                           | Operational Semantics        |   Assertion/Note/Pre/Post-condition    |
   +================================+=======================================+==============================+========================================+
   |``r += n``                      | ``X&``                                |``{ Distance m = n; if (m >=  |                                        |
   |                                |                                       |0) while (m--) ++r; else while|                                        |
   |                                |                                       |(m++) --r; return r; }``      |                                        |
   +--------------------------------+---------------------------------------+------------------------------+----------------------------------------+
   |      ``a + n``, ``n + a``      | ``X``                                 |``{ X tmp = a; return tmp +=  |                                        |
   |                                |                                       |n; }``                        |                                        |
   +--------------------------------+---------------------------------------+------------------------------+----------------------------------------+
   |``r -= n``                      | ``X&``                                |``return r += -n``            |                                        |
   +--------------------------------+---------------------------------------+------------------------------+----------------------------------------+
   |``a - n``                       | ``X``                                 |``{ X tmp = a; return tmp -=  |                                        |
   |                                |                                       |n; }``                        |                                        |
   +--------------------------------+---------------------------------------+------------------------------+----------------------------------------+
   |``b - a``                       |``Distance``                           |``a < b ?  distance(a,b) :    |pre: there exists a value ``n`` of      |
   |                                |                                       |-distance(b,a)``              |``Distance`` such that ``a + n == b``.  |
   |                                |                                       |                              |``b == a + (b - a)``.                   |
   +--------------------------------+---------------------------------------+------------------------------+----------------------------------------+
   |``a[n]``                        |convertible to T                       |``*(a + n)``                  |Not required to return an lvalue        |
   +--------------------------------+---------------------------------------+------------------------------+----------------------------------------+
   |``a[n] = t``                    |convertible to T                       |``*(a + n) = t``              |Not required to return an lvalue        |
   +--------------------------------+---------------------------------------+------------------------------+----------------------------------------+
   |``a < b``                       |convertible to ``bool``                |``b - a > 0``                 |``<`` is a total ordering relation      |
   +--------------------------------+---------------------------------------+------------------------------+----------------------------------------+
   |``a > b``                       |convertible to ``bool``                |``b < a``                     |``>`` is a total ordering relation      |
   +--------------------------------+---------------------------------------+------------------------------+----------------------------------------+
   |``a >= b``                      |convertible to ``bool``                |``!(a < b)``                  |                                        |
   +--------------------------------+---------------------------------------+------------------------------+----------------------------------------+
   |``a <= b``                      |convertible to ``bool``                |``!(a > b)``                  |                                        |
   +--------------------------------+---------------------------------------+------------------------------+----------------------------------------+
   | ``traversal_category<X>::type``|                                       |                              |Convertible to                          |
   |                                |                                       |                              |``random_access_traversal_iterator_tag``|
   +--------------------------------+---------------------------------------+------------------------------+----------------------------------------+



Addition to [lib.iterator.synopsis]
===================================

::

  // lib.iterator.traits, traits and tags
  template <class Iterator> struct access_category;
  template <class Iterator> struct traversal_category;

  template <class AccessTag, class TraversalTag>
  struct iterator_tag : <appropriate old category> {
    typedef AccessTag access;
    typedef TraversalTag traversal;
  };

  struct readable_iterator_tag { };
  struct writable_iterator_tag { };
  struct swappable_iterator_tag { };
  struct readable_writable_iterator_tag { };
  struct readable_lvalue_iterator_tag { };
  struct writable_lvalue_iterator_tag
    : virtual public readable_writable_iterator_tag,
      virtual public readable_lvalue_iterator_tag { };

  struct incrementable_iterator_tag { };
  struct single_pass_iterator_tag : public incrementable_iterator_tag { };
  struct forward_traversal_tag : public single_pass_iterator_tag { };
  struct bidirectional_traversal_tag : public forward_traversal_tag { };
  struct random_access_traversal_tag : bidirectional_traversal_tag { };

  struct null_category_tag { };
  struct input_output_iterator_tag : input_iterator_tag, output_iterator_tag {};


Addition to [lib.iterator.traits]
=================================

The ``iterator_tag`` class template is an iterator category tag that
encodes the access and traversal tags in addition to being compatible
with the original iterator tags. The ``iterator_tag`` class inherits
from one of the original iterator tags according to the following
pseudo-code.

::

   inherit-category(access-tag, traversal-tag) {
     if (access-tag is convertible to readable_lvalue_iterator_tag
         or access-tag is convertible to writable_lvalue_iterator_tag) {
       if (traversal-tag is convertible to random_access_traversal_tag)
         return random_access_iterator_tag;
       else if (traversal-tag is convertible to bidirectional_traversal_tag)
         return bidirectional_iterator_tag;
       else if (traversal-tag is convertible to forward_traversal_tag)
         return forward_iterator_tag;
       else
         return null_category_tag;
     } else if (access-tag is convertible to readable_writable_iterator_tag
                and traversal-tag is convertible to single_pass_iterator_tag)
       return input_output_iterator_tag;
     else if (access-tag is convertible to readable_iterator_tag
              and traversal-tag is convertible to single_pass_iterator_tag)
       return input_iterator_tag;
     else if (access-tag is convertible to writable_iterator_tag
              and traversal-tag is convertible to incrementable_iterator_tag)
       return output_iterator_tag;
     else
       return null_category_tag
   }


The ``access_category`` and ``traversal_category`` class templates are
traits classes. For iterators whose
``iterator_traits<Iter>::iterator_category`` type is ``iterator_tag``,
the ``access_category`` and ``traversal_category`` traits access the
``access`` and ``traversal`` member types within ``iterator_tag``.
For iterators whose ``iterator_traits<Iter>::iterator_category`` type
is not ``iterator_tag`` and instead is a tag convertible to one of the
original tags, an appropriate traversal and access tags are deduced.

::

  template <class Iterator>
  struct access_category {
    // pseudo code
    cat = iterator_traits<Iterator>::iterator_category;
    if (cat == iterator_tag<Access,Traversal>)
      return Access;
    else if (cat is convertible to forward_iterator_tag)
      if (iterator_traits<Iterator>::reference is a const reference)
        return readable_lvalue_iterator_tag;
      else
        return writable_lvalue_iterator_tag;
    else if (cat is convertible to input_iterator_tag)
      return readable_iterator_tag;
    else if (cat is convertible to output_iterator_tag)
      return writable_iterator_tag;
    else
      return null_category_tag;
  };

  template <class Iterator>
  struct traversal_category {
    // pseudo code
    cat = iterator_traits<Iterator>::iterator_category;
    if (cat == iterator_tag<Access,Traversal>)
      return Traversal;
    else if (cat is convertible to random_access_iterator_tag)
      return random_access_traversal_tag;
    else if (cat is convertible to bidirectional_iterator_tag)
      return bidirectional_traversal_tag;
    else if (cat is convertible to forward_iterator_tag)
      return forward_traversal_tag;
    else if (cat is convertible to input_iterator_tag)
      return single_pass_iterator_tag;
    else if (cat is convertible to output_iterator_tag)
      return incrementable_iterator_tag;
    else
      return null_category_tag;
  };

The following specializations provide the access and traversal
categories for pointer types.

::

  template <typename T>
  struct access_category<const T*>
  {
    typedef readable_lvalue_iterator_tag type;
  };
  template <typename T>
  struct access_category<T*>
  {
    typedef writable_lvalue_iterator_tag type;
  };

  template <typename T>
  struct traversal_category<T*>
  {
    typedef random_access_traversal_tag type;
  };



..
 LocalWords:  Abrahams Siek Witt const bool Sutter's WG int UL LI href Lvalue
 LocalWords:  ReadableIterator WritableIterator SwappableIterator cv pre iter
 LocalWords:  ConstantLvalueIterator MutableLvalueIterator CopyConstructible
 LocalWords:  ForwardTraversalIterator BidirectionalTraversalIterator lvalue
 LocalWords:  RandomAccessTraversalIterator dereferenceable Incrementable tmp
 LocalWords:  incrementable xxx min prev inplace png oldeqnew AccessTag struct
 LocalWords:  TraversalTag typename
