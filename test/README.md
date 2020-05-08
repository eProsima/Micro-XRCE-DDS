Overview
========

Meta-repository used by Jenkins CI and developers to check the interoperability between [Micro XRCE-DDS Client](https://github.com/eProsima/Micro-XRCE-DDS-Client) (*uClient*) and [Micro XRCE-DDS Agent](https://github.com/eProsima/Micro-XRCE-DDS-Agent) (*uAgent*) is ok.

Usage
=====

This CMake meta-project is designed following the *SuperBuild* approach, that is, 
the external dependencies, in this case *uClient* and *uAgent*, are added as CMake ExternalProjects on a first build step,
them the main project is build taking into account the external dependencies.
This meta-project expects two cmake arguments:

* CLIENT_BRANCH: uClient's branch to be tested.
* AGENT_BRANCH: uAgent's branch to be tested.
