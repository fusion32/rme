# Convert all material xml files in the current working directory to use client ids.

import argparse, json, os, sys, xml.etree.ElementTree as ET

def convert_id_attrib(node, attrib, item_map, verbose):
    old_id = node.get(attrib)
    if old_id != None:
        new_id = item_map.get(old_id, "0")
        node.set(attrib, new_id)
        if old_id not in item_map:
            print("WARNING: invalid conversion for <{0} \"{1}\"=\"{2}\">".format(node.tag, attrib, old_id))
        elif verbose:
            print("<{0} \"{1}\"=\"{2}\"> => <{0} \"{1}\"=\"{3}\">".format(node.tag, attrib, old_id, new_id))

def rename_attrib(node, old_attrib, new_attrib):
    if old_attrib in node.attrib:
        node.attrib[new_attrib] = node.attrib.pop(old_attrib)

def process_file(input_path, output_path, item_map, verbose = False):
    tree = ET.parse(input_path)
    root = tree.getroot()

    # TODO(fusion): Some tileset nodes use ranges (fromid, toid) which may not map exactly.
    for node in root.iter():
        if node.tag == "item" or node.tag == "carpet" or node.tag == "door":
            convert_id_attrib(node, "id", item_map, verbose)
            convert_id_attrib(node, "fromid", item_map, verbose)
            convert_id_attrib(node, "toid", item_map, verbose)
        elif node.tag == "brush":
            rename_attrib(node, "server_lookid", "lookid")
            convert_id_attrib(node, "lookid", item_map, verbose)
        elif node.tag == "borderitem":
            convert_id_attrib(node, "item", item_map, verbose)
        elif node.tag == "border":
            convert_id_attrib(node, "ground_equivalent", item_map, verbose)
        elif node.tag == "replace_border" or node.tag == "replace_item":
            convert_id_attrib(node, "with", item_map, verbose)

    os.makedirs(os.path.dirname(output_path), mode = 0o755, exist_ok = True)
    tree.write(output_path, encoding="utf-8", xml_declaration=True)

def process_dir(input_path, output_path, item_map, verbose = False):
    output_ext = ".out.xml" if input_path == output_path else ".xml"
    for filename in os.listdir(input_path):
        stem, ext = os.path.splitext(filename)
        if ext == ".xml":
            process_file(
                input_path  = os.path.join(input_path, stem + ".xml"),
                output_path = os.path.join(output_path, stem + output_ext),
                item_map    = item_map,
                verbose     = verbose)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(prog = "cvtmaterials")
    parser.add_argument("-v", "--verbose",
                        action = "store_true")
    parser.add_argument("-d", "--direction",
                        choices = ["forward", "reverse"],
                        default = "forward")
    parser.add_argument("item_map")

    args = parser.parse_args()

    with open(args.item_map, "r") as fp:
        item_map = json.load(fp)

    # make sure the item map is using string keys and values
    if args.direction == "forward":
        item_map = {str(k): str(v) for k, v in item_map.items()}
    else:
        item_map = {str(v): str(k) for k, v in item_map.items()}

    cwd = os.getcwd()
    process_dir(
        input_path  = cwd,
        output_path = os.path.join(cwd, "out"),
        item_map    = item_map,
        verbose     = args.verbose)


