package eoma.system.automation;

import javax.swing.*;
import javax.xml.parsers.*;
import org.w3c.dom.*;
import java.awt.*;
import java.awt.event.*;
import java.io.*;
import java.time.LocalTime;
import java.util.HashMap;
import java.util.Map;

public class Automation {

    private JLabel lblOutput;
    private Map<String, JButton> buttons = new HashMap<>();

    public static void main(String[] args) {
        SwingUtilities.invokeLater(() -> new Automation().createAndShowGUI());
    }

    private void createAndShowGUI() {
        // Parse XML
        File xmlFile = new File("layout.xml");
        if (!xmlFile.exists()) {
            JOptionPane.showMessageDialog(null, "layout.xml não encontrado!");
            return;
        }

        Document doc;
        try {
            DocumentBuilderFactory dbFactory = DocumentBuilderFactory.newInstance();
            DocumentBuilder dBuilder = dbFactory.newDocumentBuilder();
            doc = dBuilder.parse(xmlFile);
            doc.getDocumentElement().normalize();
        } catch (Exception e) {
            e.printStackTrace();
            JOptionPane.showMessageDialog(null, "Erro ao ler layout.xml");
            return;
        }

        // Window
        Node windowNode = doc.getElementsByTagName("window").item(0);
        Element windowEl = (Element) windowNode;
        String title = windowEl.getAttribute("title");
        int width = Integer.parseInt(windowEl.getAttribute("width"));
        int height = Integer.parseInt(windowEl.getAttribute("height"));

        JFrame frame = new JFrame(title);
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        frame.setSize(width, height);
        frame.setLayout(null);

        // Iterate children
        NodeList nodes = windowEl.getChildNodes();
        for (int i = 0; i < nodes.getLength(); i++) {
            Node n = nodes.item(i);
            if (n.getNodeType() != Node.ELEMENT_NODE) continue;
            Element e = (Element) n;

            String tag = e.getTagName();
            if (tag.equals("button")) {
                JButton btn = new JButton(e.getAttribute("text"));
                int x = Integer.parseInt(e.getAttribute("x"));
                int y = Integer.parseInt(e.getAttribute("y"));
                int w = Integer.parseInt(e.getAttribute("width"));
                int h = Integer.parseInt(e.getAttribute("height"));
                btn.setBounds(x, y, w, h);
                String cmd = e.getAttribute("command");

                btn.addActionListener(ev -> executeCommand(cmd));
                frame.add(btn);
                buttons.put(e.getAttribute("id"), btn);

            } else if (tag.equals("label")) {
                lblOutput = new JLabel(e.getAttribute("text"));
                int x = Integer.parseInt(e.getAttribute("x"));
                int y = Integer.parseInt(e.getAttribute("y"));
                int w = Integer.parseInt(e.getAttribute("width"));
                int h = Integer.parseInt(e.getAttribute("height"));
                lblOutput.setBounds(x, y, w, h);
                lblOutput.setVerticalAlignment(SwingConstants.TOP);
                lblOutput.setForeground(Color.BLUE);
                frame.add(lblOutput);
            }
        }

        frame.setLocationRelativeTo(null);
        frame.setVisible(true);
    }

    private void executeCommand(String command) {
        try {
            if (command.equals("echo hora")) {
                lblOutput.setText("Hora atual: " + LocalTime.now().withNano(0).toString());
                return;
            }

            // Executa comando no sistema
            ProcessBuilder pb = new ProcessBuilder("/bin/bash", "-c", command);
            pb.redirectErrorStream(true);
            Process process = pb.start();

            // Lê saída
            BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()));
            StringBuilder output = new StringBuilder();
            String line;
            while ((line = reader.readLine()) != null) {
                output.append(line).append("<br>");
            }
            lblOutput.setText("<html>" + output.toString() + "</html>");

        } catch (IOException ex) {
            lblOutput.setText("Erro ao executar comando: " + command);
            ex.printStackTrace();
        }
    }
}
