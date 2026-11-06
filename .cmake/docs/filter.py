import os
import sys
import xml.etree.ElementTree as ET


def has_brief(elem: ET.Element) -> bool:
    brief = elem.find("briefdescription")
    return brief is not None and "".join(brief.itertext()).strip() != ""


def filter_doxygen_xml(xml_dir: str) -> None:
    index_path = os.path.join(xml_dir, "index.xml")
    if not os.path.isfile(index_path):
        return

    index_tree = ET.parse(index_path)
    index_root = index_tree.getroot()

    # Collect all defined groups (kind="group") and the classes/structs inside them
    allowed_refids: set[str] = set()
    for compound in index_root.findall("compound"):
        if compound.get("kind") == "group":
            refid = compound.get("refid", "")
            allowed_refids.add(refid)
            group_xml = os.path.join(xml_dir, f"{refid}.xml")
            if os.path.isfile(group_xml):
                gtree = ET.parse(group_xml)
                for inner in gtree.findall(".//innerclass"):
                    inner_ref = inner.get("refid")
                    if inner_ref:
                        allowed_refids.add(inner_ref)
                for inner_grp in gtree.findall(".//innergroup"):
                    inner_ref = inner_grp.get("refid")
                    if inner_ref:
                        allowed_refids.add(inner_ref)

    # Verify grouped classes/structs actually have a @brief, and prune undocumented members
    verified_refids: set[str] = set()
    for refid in allowed_refids:
        cpath = os.path.join(xml_dir, f"{refid}.xml")
        if not os.path.isfile(cpath):
            continue
        ctree = ET.parse(cpath)
        croot = ctree.getroot()
        cdef = croot.find("compounddef")
        if cdef is None:
            continue

        # Keep all groups; for classes/structs, require an explicit @brief
        if cdef.get("kind") != "group" and not has_brief(cdef):
            continue

        # Remove undocumented members (no @brief) from the compound
        for section in list(cdef.findall("sectiondef")):
            for member in list(section.findall("memberdef")):
                if not has_brief(member):
                    section.remove(member)
            if len(section.findall("memberdef")) == 0:
                cdef.remove(section)

        ctree.write(cpath, encoding="utf-8", xml_declaration=True)
        verified_refids.add(refid)

    # Remove everything else from index.xml and delete unused .xml files
    for compound in list(index_root.findall("compound")):
        refid = compound.get("refid", "")
        if refid not in verified_refids:
            index_root.remove(compound)
            cpath = os.path.join(xml_dir, f"{refid}.xml")
            if os.path.isfile(cpath):
                os.remove(cpath)

    index_tree.write(index_path, encoding="utf-8", xml_declaration=True)


if __name__ == "__main__":
    filter_doxygen_xml(sys.argv[1])
