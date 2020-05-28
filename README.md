# eProsima Micro XRCE-DDS

[![FIWARE Robotics](https://nexus.lab.fiware.org/static/badges/chapters/robotics.svg)](https://www.fiware.org/developers/catalogue/)
[![License](https://img.shields.io/github/license/eProsima/Micro-XRCE-DDS.svg)](https://github.com/eProsima/Micro-XRCE-DDS/blob/master/LICENSE)
[![Releases](https://img.shields.io/github/release/eProsima/Micro-XRCE-DDS.svg)](https://github.com/eProsima/Micro-XRCE-DDS/releases)
[![Issues](https://img.shields.io/github/issues/eProsima/Micro-XRCE-DDS.svg)](https://github.com/eProsima/Micro-XRCE-DDS/issues)
[![Forks](https://img.shields.io/github/forks/eProsima/Micro-XRCE-DDS.svg)](https://github.com/eProsima/Micro-XRCE-DDS/network/members)
[![Stars](https://img.shields.io/github/stars/eProsima/Micro-XRCE-DDS.svg)](https://github.com/eProsima/Micro-XRCE-DDS/stargazers)
<br/>
[![Documentation badge](https://img.shields.io/readthedocs/micro-xrce-dds.svg)](https://micro-xrce-dds.readthedocs.io)
![Status](https://nexus.lab.fiware.org/repository/raw/public/badges/statuses/incubating.svg)

<a href="http://www.eprosima.com"><img src="https://encrypted-tbn3.gstatic.com/images?q=tbn:ANd9GcSd0PDlVz1U_7MgdTe0FRIWD0Jc9_YH-gGi0ZpLkr-qgCI6ZEoJZ5GBqQ" align="left" hspace="8" vspace="2" width="100" height="100" ></a>

*eProsima Micro XRCE-DDS* is a software solution which allows to communicate eXtremely Resource Constrained Environments (XRCEs) with an existing DDS network.
 This implementation complies with the specification proposal, "eXtremely Resource Constrained Environments DDS (DDS-XRCE)" submitted to the Object Management Group (OMG) consortium.

*Micro XRCE-DDS* implements a client-server protocol to enable resource-constrained devices (clients) to take part in DDS communications.
*Micro XRCE-DDS Agent* (server) makes possible this communication.
The *Micro XRCE-DDS Agent* acts on behalf of the *Micro XRCE-DDS Clients* and enables them to take part as DDS publishers and/or subscribers in the DDS Global Data Space.
*Micro XRCE-DDS* provides both, a plug and play *Micro XRCE-DDS Agent* and an API layer which allows you to implement your *Micro XRCE-DDS Clients*.

<p align="center"> <img src="docs/xrcedds_architecture.png" alt="Image"/> </p>

This repository contains the totality of *Micro XRCE-DDS* products:

<p align="center"> <img src="docs/general_architecture.png" alt="Image"/> </p>

This project is part of [FIWARE](https://www.fiware.org/). For more information check the FIWARE Catalogue entry for
[Robotics](https://github.com/Fiware/catalogue/tree/master/robotics).

## Documentation

You can access Micro XRCE-DDS documentation online, which is hosted on Read the Docs.

* [Start Page](http://micro-xrce-dds.readthedocs.io)
* [Installation manual](http://micro-xrce-dds.readthedocs.io/en/latest/installation.html)
* [User manual](http://micro-xrce-dds.readthedocs.io/en/latest/introduction.html)

## Getting Help

If you need support you can reach us by mail at `support@eProsima.com` or by phone at `+34 91 804 34 48`.
