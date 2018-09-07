import com.cesnet.*;
import com.cesnet.Module;
import com.huawei.vncalarm.util.FileUtil;

import static com.cesnet.LYD_FORMAT.LYD_XML;
import static com.cesnet.yangConstants.LYD_OPT_CONFIG;

public class ProcessTree {

    public void printNode(Data_Node node){
        if (node != null){
            System.out.println("tree_dfs\n");
            vectorData_Node data_list= node.tree_dfs();
            for (int i = 0; i < data_list.size(); i++) {
                Data_Node elem = data_list.get(i);
                System.out.println("name: " + elem.schema().name() + " type: " + elem.schema().nodetype());
            }

            System.out.println("\nChild of " + node.schema().name() + " is: " + node.child().schema().name() + "\n");

            System.out.println("tree_for\n");

            data_list = node.child().child().tree_dfs();
            for (int i = 0; i < data_list.size(); i++) {
                Data_Node elem = data_list.get(i);
                System.out.println("child of " + node.child().schema().name() + " is: " + elem.schema().name() + " type: " + elem.schema().nodetype());
            }


            System.out.println("\n schema tree_dfs\n");
            vectorSchema_Node schema_list = node.schema().tree_dfs();
            for (int i = 0; i < schema_list.size(); i++) {
                Schema_Node elem = schema_list.get(i);
                System.out.println("schema name " + elem.name() + " type " + elem.nodetype());

            }
        }
    }

    public static void main(String[] args) {
        System.loadLibrary("yangJava");
        com.cesnet.Context ctx = new com.cesnet.Context("/etc/sysrepo/yang");
        Module module = ctx.get_module("turing-machine");
        if (module != null){
            System.out.println(module.name());
        }else {
            ctx.load_module("turing-machine");
        }

        ProcessTree processTree = new ProcessTree();
        Data_Node node = ctx.parse_data_path("/etc/sysrepo/data/turing-machine.startup", LYD_XML, LYD_OPT_CONFIG);

        Data_Node mem_node = ctx.parse_data_mem(
                FileUtil.readToString("/etc/sysrepo/data/turing-machine.startup"),
                LYD_XML, LYD_OPT_CONFIG);

        processTree.printNode(node);
        System.out.println("------------------------------------");
        processTree.printNode(mem_node);

    }
}
