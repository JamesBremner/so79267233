# Optimizing kid's satisfaction

This application tackles the impossible task of satisfying a bunch of kids by giving them candy.

# Problem Specification

## Inputs:

n types of candies, each with a quantity q[i].

m kids, each with:

A satisfaction matrix s[j][i]: the satisfaction a kid j gets per candy of type i.

A satisfaction threshold c[j]: the minimum total satisfaction needed for a kid j to be satisfied.

Special "universal candies" that contribute 1 unit of satisfaction to any kid and are theoretically unlimited.

## Objective:

Satisfy every kid's threshold c[j] by distributing candies (and special candies if needed).
Minimize the total number of special candies used.

## Constraints:

The total amount of each candy type distributed cannot exceed its quantity q[i].
Each kid can receive any number of candies from each type.

## Output:

The distribution of candies.
